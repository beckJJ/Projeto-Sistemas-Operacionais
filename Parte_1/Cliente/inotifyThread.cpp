#include "inotifyThread.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <utility>
#include "../Common/package.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/defines.hpp"

thread_local int fd = -1;

void sigterm_handler(int)
{
    // fd usado pelo inotify
    if (fd != -1)
    {
        // Fecha socket
        close(fd);
    }

    // Encerra thread
    pthread_exit(NULL);
}

void handleMove(std::vector<UserChangeEvent> &incompleteEvents, std::vector<UserChangeEvent> &completeEvents, const char *filename, uint32_t cookie, bool isMovedFrom)
{
    for (size_t index = 0; index < incompleteEvents.size(); index++)
    {
        if (incompleteEvents[index].cookie == cookie)
        {
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

            completeEvents.push_back(userEvent);
            return;
        }
    }

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
inotifyThread(void *threadArg)
{
    int socket_id = ((ThreadArg *)threadArg)->socket_id;
    uint8_t device_id = ((ThreadArg *)threadArg)->device_id;
    char *username = ((ThreadArg *)threadArg)->username;

    //  Vetor usado para registrar eventos MOVED_ ainda não resolvido para renomeações, quando
    //    completado será adicionado ao vetor completeUserEvents
    std::vector<UserChangeEvent> incompleteUserEvents;
    // UserChangeEvent já completados
    std::vector<UserChangeEvent> completeUserEvents;

    std::vector<char> fileContentBuffer;

    std::string path_to_watch = PREFIXO_DIRETORIO;
    path_to_watch.append(username);

    fd = inotify_init();

    if (fd == -1)
    {
        perror("Erro ao inicializar o inotify");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(fd, path_to_watch.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

    if (wd == -1)
    {
        perror("Erro ao adicionar o diretório ao inotify");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];

    while (1)
    {
        ssize_t len = read(fd, buffer, sizeof(buffer));

        if (len == -1)
        {
            perror("Erro ao ler eventos do inotify");
            close(fd);
            exit(EXIT_FAILURE);
        }

        for (char *ptr = buffer; ptr < buffer + len;)
        {
            struct inotify_event *event = (struct inotify_event *)ptr;
            std::string filename = event->name;

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

            // Envia eventos
            for (auto userEvent : completeUserEvents)
            {
                auto package = Package(PackageChangeEvent(userEvent.changeEvent, device_id, userEvent.movedFrom.c_str(), userEvent.movedTo.c_str()));

                if (write_package_to_socket(socket_id, package, fileContentBuffer))
                {
                    printf("Nao foi possivel enviar evento.\n");
                    exit(EXIT_FAILURE);
                }
            }

            ptr += sizeof(struct inotify_event) + event->len;

            // Eventos já foram enviados, limpa vetor de eventos
            completeUserEvents.clear();
        }
    }

    close(fd);

    return NULL;
}
