#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "serverThread.hpp"
#include "../Common/defines.hpp"
#include "../Common/package.hpp"
#include "../Common/package_file.hpp"

#include <pthread.h>
#include <threads.h>

#include "../Common/package_functions.hpp"
#include "../Common/functions.hpp"
#include <string.h>
#include "mainConnectionLoop.hpp"
#include "eventConnectionLoop.hpp"

extern DeviceManager deviceManager;

thread_local pid_t tid = 0;
thread_local int socket_id = -1;

// Processa pacotes iniciais de identificação
int connectUser(int socket_id, ConnectionType &connectionType, std::string &username, User *&user, uint8_t &deviceID)
{
    auto package = Package();
    std::vector<char> fileContentBuffer;

    if (read_package_from_socket(socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao ler primeiro pacote do usuario\n", tid);
        return 1;
    }

    // O primeiro pacote enviado pelo usuário deve ser INITAL_USER_INDENTIFICATION
    if (package.package_type != INITAL_USER_INDENTIFICATION)
    {
        printf("[tid: %d] Pacote inicial do usuario nao e identificacao: 0x%02x\n", tid, (uint8_t)package.package_type);
        return 1;
    }

    username = std::string(package.package_specific.userIdentification.user_name);
    connectionType = package.package_specific.userIdentification.connectionType;
    deviceID = package.package_specific.userIdentification.deviceID;

    std::optional<DeviceConnectReturn> deviceConnectReturn;

    // Conecta-se dependendo do tipo de conexão desejada
    if (connectionType == MAIN_CONNECTION)
    {
        printf("[tid: %d] Connecting main device \n", tid);
        deviceConnectReturn = deviceManager.connect_main(socket_id, username);
    }
    else
    {
        printf("[tid: %d] Connecting event device\n", tid);
        deviceConnectReturn = deviceManager.connect_event(socket_id, username, deviceID);
    }

    // Conexão rejeitada
    if (!deviceConnectReturn.has_value())
    {
        printf("[tid: %d] Nova conexao do usuario \"%s\" rejeitada.\n", tid, username.c_str());

        // Responde ao usuário indicando rejeição da conexão
        package = Package(PackageUserIndentificationResponse(REJECTED, 0));
        write_package_to_socket(socket_id, package, fileContentBuffer);
        return 1;
    }

    user = deviceConnectReturn.value().user;
    deviceID = deviceConnectReturn.value().deviceID;
    connectionType = deviceConnectReturn.value().connectionType;

    printf("[tid: %d] Nova conexão do usuario \"%s\" no dispositivo 0x%02x.\n",
           tid,
           username.c_str(),
           (uint8_t)deviceID);

    // Responde ao usuário indicando sucesso da conexão
    package = Package(PackageUserIndentificationResponse(ACCEPTED, deviceID));

    if (write_package_to_socket(socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao enviar resposta inicial ao usuario.\n", tid);
        return 1;
    }

    return 0;
}

void *serverThread(void *arg)
{
    socket_id = ((ServerThreadArg *)arg)->socket_id;

    std::string username;
    User *user;
    uint8_t deviceID = 0;
    ConnectionType connectionType;

#ifndef __APPLE__
    tid = gettid();
#else
    // macOS nao tem gettid()
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    tid = (pid_t)tid;
#endif

    std::vector<char> fileContentBuffer;

    // Tentar conectar-se como uma conexão de um dispositivo do usuário
    if (connectUser(socket_id, connectionType, username, user, deviceID))
    {
        close(socket_id);
        return NULL;
    }

    switch (connectionType)
    {
    case MAIN_CONNECTION:
        printf("[tid: %d] Thread registrada como conexao principal.\n", tid);
        mainConnectionLoop(socket_id, tid, username, user);
        break;
    case EVENT_CONNECTION:
        printf("[tid: %d] Thread registrada como conexao de eventos.\n", tid);
        eventConnectionLoop(socket_id, tid, username, user);
        break;
    default:
        printf("[tid: %d] Unknown connection type: 0x%02x\n", tid, (uint8_t)connectionType);
        break;
    }

    printf("[tid: %d] Thread disconnecting.\n", tid);

    // Desconecta o dispositivo, mainConnectionLoop e eventConnectionLoop só retornam no caso de não
    //   ser possível ler pacotes
    deviceManager.disconnect(username, deviceID);

    return NULL;
}
