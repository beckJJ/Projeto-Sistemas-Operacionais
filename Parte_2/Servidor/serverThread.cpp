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
#include "serverLoop.hpp"
#include "connections.hpp"
#include <arpa/inet.h>

extern DeviceManager deviceManager;
extern ActiveConnections_t activeConnections;

thread_local pid_t tid = 0;
thread_local Client_t client = {-1, {0, 0, 0, 0}};


// Processa pacotes iniciais de identificação
int connectUser(Client_t client, std::string &username, User *&user, Device *&device, uint8_t &deviceID)
{
    auto package = Package();
    std::vector<char> fileContentBuffer;

    if (read_package_from_socket(client.socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao ler primeiro pacote do usuario\n", tid);
        return 1;
    }

    // O primeiro pacote enviado pelo usuário deve ser INITAL_USER_IDENTIFICATION
    if (package.package_type != INITAL_USER_IDENTIFICATION)
    {
        printf("[tid: %d] Pacote inicial do usuario nao e identificacao: 0x%02x\n", tid, (uint8_t)package.package_type);
        return 1;
    }

    username = std::string(package.package_specific.userIdentification.user_name);
    deviceID = package.package_specific.userIdentification.deviceID;

    std::optional<DeviceConnectReturn> deviceConnectReturn;

    // Tenta conectar como um dispositivo do usuário
    deviceConnectReturn = deviceManager.connect(client, username);

    // Conexão rejeitada
    if (!deviceConnectReturn.has_value())
    {
        printf("[tid: %d] Nova conexao do usuario \"%s\" rejeitada.\n", tid, username.c_str());

        // Responde ao usuário indicando rejeição da conexão
        package = Package(PackageUserIdentificationResponse(REJECTED, 0));
        write_package_to_socket(client.socket_id, package, fileContentBuffer);
        return 1;
    }

    user = deviceConnectReturn.value().user;
    deviceID = deviceConnectReturn.value().deviceID;
    device = deviceConnectReturn.value().device;

    printf("[tid: %d] Nova conexão do usuario \"%s\", dispositivo: 0x%02x.\n",
           tid,
           username.c_str(),
           (uint8_t)deviceID);

    // Responde ao usuário indicando sucesso da conexão
    package = Package(PackageUserIdentificationResponse(ACCEPTED, deviceID));

    if (write_package_to_socket(client.socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao enviar resposta inicial ao usuario.\n", tid);
        return 1;
    }

    return 0;
}

void *serverThread(void *arg)
{
    client.socket_id = ((ServerThreadArg *)arg)->socket_id;
    client.address = ((ServerThreadArg *)arg)->socket_address;

    std::string username;
    User *user;
    Device *device;
    uint8_t deviceID = 0;

#ifndef __APPLE__
    tid = gettid();
#else
    // macOS nao tem gettid()
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    tid = (pid_t)tid;
#endif

    std::vector<char> fileContentBuffer;

    // Tentar conectar-se como um dispositivo do usuário
    if (connectUser(client, username, user, device, deviceID))
    {
        close(client.socket_id);
        return NULL;
    }

    serverLoop(client.socket_id, tid, username, user, device);

    // Desconecta o dispositivo atual, serverLoop só retorna no caso de não ser possível ler pacotes
    printf("[tid: %d] Disconnecting thread.\n", tid);
    deviceManager.disconnect(username, deviceID, client);

    // socket_id será fechado no destrutor de Device

    return NULL;
}
