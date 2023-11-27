#include "eventThread.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <utility>
#include "../Common/package.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_file.hpp"
#include "../Common/defines.hpp"
#include "DadosConexao.hpp"
#include <signal.h>
#include <string.h>

extern DadosConexao dados_conexao;
extern PackageChangeEvent previousSyncedChangeEvent;
extern pthread_mutex_t previousSyncedChangeEventLock;
extern pthread_mutex_t event_connection_socket_lock;
thread_local int fd = -1;

// Encerra thread
void exitEventThread(void)
{
    // fd usado pelo inotify
    if (fd != -1)
    {
        close(fd);
    }

    // Indica que thread não está mais executando
    dados_conexao.event_thread = std::nullopt;

    pthread_exit(NULL);
}

// Lida com ventos MOVED_TO e MOVED_FROM, caso cookie não seja encontrado, adicionará um evento em
//   incompleteEvents, caso o cookie seja encontrado o evento em incompleteEvents será removido
//   e adicionado em completeEvents com a modificação necessária
void handleMove(std::vector<UserChangeEvent> &incompleteEvents, std::vector<UserChangeEvent> &completeEvents, const char *filename, uint32_t cookie, bool isMovedFrom)
{
    for (size_t index = 0; index < incompleteEvents.size(); index++)
    {
        // Verifica cookie
        if (incompleteEvents[index].cookie == cookie)
        {
            // Obtém evento incompleto e o remove de incompleteEvents
            UserChangeEvent userEvent = incompleteEvents[index];
            incompleteEvents.erase(incompleteEvents.begin() + index);

            if (isMovedFrom)
            {
                userEvent.movedFrom = std::string(filename);
            }
            else
            {
                userEvent.movedTo = std::string(filename);
            }

            // Adiciona evento em completeEvents
            completeEvents.push_back(userEvent);
            return;
        }
    }

    // Adiciona evento em inCompleteEvents
    if (isMovedFrom)
    {
        incompleteEvents.push_back({FILE_RENAME, std::string(filename), std::string(), cookie});
    }
    else
    {
        incompleteEvents.push_back({FILE_RENAME, std::string(), std::string(filename), cookie});
    }
}

void *
eventThread(void *)
{
    //  Vetor usado para registrar eventos MOVED_ ainda não resolvido para renomeações, quando
    //    completado o evento será removido e adicionado ao vetor completeUserEvents
    std::vector<UserChangeEvent> incompleteUserEvents;
    // Eventos já completados que devem ser enviados
    std::vector<UserChangeEvent> completeUserEvents;

    std::vector<char> fileContentBuffer;

    std::string path_to_watch = PREFIXO_DIRETORIO;
    path_to_watch.append(dados_conexao.nome_usuario);

    // Inicializa inotify
    fd = inotify_init();

    if (fd == -1)
    {
        perror("[event] Erro ao inicializar o inotify");
        exitEventThread();
    }

    // Observa diretório sync dir local
    int wd = inotify_add_watch(fd, path_to_watch.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

    if (wd == -1)
    {
        perror("[event] Erro ao adicionar o diretório ao inotify");
        exitEventThread();
    }

    // Buffer para eventos do inotify
    char buffer[1024];

    while (1)
    {
        ssize_t len = read(fd, buffer, sizeof(buffer));

        if (len == -1)
        {
            perror("[event] Erro ao ler eventos do inotify");
            exitEventThread();
        }

        for (char *ptr = buffer; ptr < buffer + len;)
        {
            struct inotify_event *event = (struct inotify_event *)ptr;
            std::string filename = event->name;

            // Ignora eventos relacionados a diretórios
            if (event->mask & IN_ISDIR)
            {
                ptr += sizeof(struct inotify_event) + event->len;
                continue;
            }

            // Evento de renomeação, chama handleMove
            if (event->mask & (IN_MOVED_FROM | IN_MOVED_TO))
            {
                handleMove(incompleteUserEvents, completeUserEvents, event->name, event->cookie, event->mask & IN_MOVED_FROM);
            }
            else if (event->mask & IN_CREATE)
            {
                completeUserEvents.push_back({FILE_CREATED, std::string(event->name), std::string(""), 0});
            }
            else if (event->mask & IN_MODIFY)
            {
                completeUserEvents.push_back({FILE_MODIFIED, std::string(event->name), std::string(""), 0});
            }
            else if (event->mask & IN_DELETE)
            {
                completeUserEvents.push_back({FILE_DELETED, std::string(event->name), std::string(""), 0});
            }

            // Acessamos previousEvent para verificar se é necessário enviar evento para o servidor
            pthread_mutex_lock(&previousSyncedChangeEventLock);
            auto previousEvent = previousSyncedChangeEvent;
            pthread_mutex_unlock(&previousSyncedChangeEventLock);

            // Envia eventos completados
            for (auto userEvent : completeUserEvents)
            {
                auto changeEvent = PackageChangeEvent(userEvent.changeEvent, dados_conexao.deviceID, userEvent.movedFrom.c_str(), userEvent.movedTo.c_str());
                auto package = Package(changeEvent);

                // Ignora eventos gerados pelo inotify que foram gerados em resposta ao último evento
                //   de alteração enviado pelo servidor:
                //   READ  Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- previousEvent
                //   WRITE Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- changeEvent
                if ((previousEvent.deviceID != changeEvent.deviceID) &&
                    (previousEvent.event == changeEvent.event) &&
                    (!strcmp(previousEvent.filename1, changeEvent.filename1)) &&
                    (!strcmp(previousEvent.filename2, changeEvent.filename2)))
                {
                    continue;
                }

                // Socket será usado
                pthread_mutex_lock(dados_conexao.event_connection_socket_lock);

                if (write_package_to_socket(dados_conexao.event_connection_socket, package, fileContentBuffer))
                {
                    printf("[event] Nao foi possivel enviar evento.\n");
                    pthread_mutex_unlock(dados_conexao.event_connection_socket_lock);
                    exitEventThread();
                }

                // Se o tipo de evento foi FILE_MODIFIED então o conteúdo do arquivo também será enviado
                if (changeEvent.event == FILE_MODIFIED)
                {
                    std::string path = path_to_watch;
                    path.append("/");
                    path.append(event->name);

                    // Envia o arquivo modificado
                    if (send_file_from_path(dados_conexao.event_connection_socket, path.c_str(), event->name))
                    {
                        printf("[event] Nao foi possivel enviar arquivo modificado \"%s\".\n", event->name);
                    }
                }

                pthread_mutex_unlock(dados_conexao.event_connection_socket_lock);
            }

            ptr += sizeof(struct inotify_event) + event->len;

            // Eventos já foram enviados, limpa vetor de eventos
            completeUserEvents.clear();
        }
    }

    exitEventThread();
}
