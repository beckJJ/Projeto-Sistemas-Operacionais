#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "servFunc.hpp"
#include "../Common/defines.hpp"
#include "../Common/package.hpp"
#include "../Common/package_commands.hpp"

#include <pthread.h>
#include <threads.h>

#include "../Common/package_functions.hpp"
#include "../Common/functions.hpp"
#include <string.h>

extern DeviceManager deviceManager;

thread_local pid_t tid;
thread_local int socket_id;

void sigterm_handler(int)
{
    printf("[tid: %d] Received SIGTERM\n", tid);

    // Fecha socket
    close(socket_id);

    // Encerra thread
    pthread_exit(NULL);
}

void *servFunc(void *arg)
{
    std::string username;
    Package package;
    std::optional<std::pair<UserDevices *, uint8_t>> deviceConnectReturn;
    UserDevices *userDevices;
    uint8_t deviceID = 0;
    pthread_t thread_self = pthread_self();

#ifndef __APPLE__
    tid = gettid();
#else
    // macOS nao tem gettid()
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    tid = (pid_t)tid;
#endif

    struct thread_arg_t *thread_arg = (struct thread_arg_t *)arg;
    std::vector<char> fileContentBuffer;

    socket_id = thread_arg->socket_id;

    // Registra sigterm_handler para SIGTERM, fecha socket e informa no terminal
    signal(SIGTERM, sigterm_handler);

    if (read_package_from_socket(socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao ler primeiro pacote do usuario\n", tid);
        goto cleanup;
    }

    if (package.package_type != INITAL_USER_INDENTIFICATION)
    {
        printf("[tid: %d] Pacote inicial do usuario nao e identificacao: 0x%02x\n", tid, (uint8_t)package.package_type);
        goto cleanup;
    }

    username = std::string(package.package_specific.userIdentification.user_name);
    deviceConnectReturn = deviceManager.connect(username, thread_self);

    if (!deviceConnectReturn.has_value())
    {
        printf("[tid: %d] Novo dispositivo do usuario \"%s\" rejeitado.\n", tid, username.c_str());
        package = Package(PackageUserIndentificationResponse(REJECTED, 0));
        write_package_to_socket(socket_id, package, fileContentBuffer);
        goto cleanup;
    }

    userDevices = deviceConnectReturn.value().first;
    deviceID = deviceConnectReturn.value().second;

    printf("[tid: %d] Novo dispositivo do usuario \"%s\" conectado.\n", tid, username.c_str());

    package = Package(PackageUserIndentificationResponse(ACCEPTED, deviceID));

    if (write_package_to_socket(socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao enviar resposta inicial ao usuario.\n", tid);
        goto cleanup_disconnect;
    }

    while (true)
    {
        if (read_package_from_socket(socket_id, package, fileContentBuffer))
        {
            printf("[tid: %d] Nao foi possivel ler pacote.\n", tid);
            break;
        }

        switch (package.package_type)
        {
        case REQUEST_FILE:
        {
            std::string path = std::string(PREFIXO_DIRETORIO_SERVIDOR);
            path.append("/");
            path.append(username);
            path.append("/");
            path.append(package.package_specific.requestFile.filename);

            // Arquivos em sync dir serão lidos
            pthread_mutex_lock(userDevices->files_lock);

            // Obtém dados do arquivo que será enviado
            auto file = userDevices->get_file(package.package_specific.requestFile.filename);

            if (file.has_value())
            {
                package = Package(PackageUploadFile(file.value()));

                if (!write_package_to_socket(socket_id, package, fileContentBuffer))
                {
                    send_file(socket_id, path.c_str());
                }
            }
            else
            {
                package = Package(PackageFileNotFound());
                write_package_to_socket(socket_id, package, fileContentBuffer);
            }

            pthread_mutex_unlock(userDevices->files_lock);

            break;
        }
        case REQUEST_FILE_LIST:
        {
            pthread_mutex_lock(userDevices->files_lock);
            send_file_list(socket_id, *userDevices->files);
            pthread_mutex_unlock(userDevices->files_lock);
            break;
        }
        case UPLOAD_FILE:
        {
            std::string path = std::string(PREFIXO_DIRETORIO_SERVIDOR);
            path.append("/");
            path.append(username);
            path.append("/");
            path.append(package.package_specific.uploadFile.name);

            // Arquivos em sync dir serão modificados
            pthread_mutex_lock(userDevices->files_lock);

            if (read_file_and_save(socket_id, path.c_str()))
            {
                printf("[tid: %d] Nao foi possivel ler arquivo enviado pelo usuario.\n", tid);
            }
            else
            {
                userDevices->add_file_or_replace(package.package_specific.uploadFile);
            }

            pthread_mutex_unlock(userDevices->files_lock);

            break;
        }
        case CHANGE_EVENT:   // inotify e propagar notificação de alteração
        case FILE_CONTENT:   // Deve ser usado após requisitar arquivo do usuario
        case FILE_NOT_FOUND: // Enviado quando requisitar arquivo atualizado pelo usuário e o mesmo ter sido já removido
            printf("[tid: %d] Ainda nao implementado.\n", tid);
            break;
        case FILE_LIST: // Não há motivos para o usuário enviar uma lista de arquivos para o servidor
            printf("[tid: %d] Pacote FILE_LIST nao deveria ter sido enviado pelo usuario.\n", tid);
            break;
        default:
            printf("[tid: %d] Codigo comunicacao desconhecido: 0x%02x\n", tid, (uint8_t)package.package_type);
            break;
        }
    }

cleanup_disconnect:
    printf("[tid: %d] Thread disconnecting.\n", tid);

    // Remove a thread da lista de dispositivos do usuário
    deviceManager.disconnect(username, deviceID);

cleanup:
    // Fecha socket
    close(socket_id);

    return NULL;
}
