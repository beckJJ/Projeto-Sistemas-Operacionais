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
#include "../Common/connections.hpp"
#include <arpa/inet.h>

extern DeviceManager deviceManager;
extern ActiveConnections_t activeConnections;
extern bool backup;

thread_local pid_t tid = 0;
thread_local Connection_t client = Connection_t(0, 0, 0xFFFF, "");

int connectToServer(Connection_t connection, std::string &username, User *&user, Device *&device, uint8_t &deviceID)
{
    auto package = Package();
    std::vector<char> fileContentBuffer;

    if (read_package_from_socket(connection.socket_id, package, fileContentBuffer)) {
        printf("[tid: %d] Erro ao ler primeiro pacote da conexao\n", tid);
        return 1;
    }
    
    switch (package.package_type) {
        case INITIAL_USER_IDENTIFICATION:
            return connectUser(connection, username, user, device, deviceID, package, fileContentBuffer);
        case INITIAL_REPLICA_MANAGER_IDENTIFICATION: // thread de ping do servidor principal com o backup
            username = "backup";
            return connectBackup(connection, deviceID, package, fileContentBuffer);
        case REPLICA_MANAGER_TRANSFER_IDENTIFICATION: // thread de transferência de arquivos do servidor principal para o backup
            username = "backup";
            return connectBackupTransfer(connection, deviceID, package, fileContentBuffer);
        default: 
            printf("[tid: %d] Pacote inicial da conexao nao e identificacao: 0x%02x\n", tid, (uint8_t)package.package_type);
            return 1;
    }
}

int connectBackupTransfer(Connection_t server, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer)
{
    deviceID = package.package_specific.replicaManagerTransferIdentification.deviceID;
    std::optional<DeviceConnectReturn> deviceConnectReturn;

    std::string username = "backup";
    deviceConnectReturn = deviceManager.connect(server, username, true);

    if (!deviceConnectReturn.has_value()) {
        printf("[tid: %d] Nova conexao do backup rejeitada.\n", tid);

        // Responde ao usuário indicando rejeição da conexão
        package = Package(PackageReplicaManagerTransferIdentificationResponse(REJECTED_RM_T, 0));
        write_package_to_socket(server.socket_id, package, fileContentBuffer);
        return 1;
    }
    deviceID = deviceConnectReturn.value().deviceID;
    printf("[tid: %d] Nova conexao de transferencia com backup, dispositivo: 0x%02x.\n", tid, (uint8_t)deviceID);

    // Responde ao backup indicando sucesso da conexão
    package = Package(PackageReplicaManagerTransferIdentificationResponse(ACCEPTED_RM_T, deviceID));

    if (write_package_to_socket(server.socket_id, package, fileContentBuffer)) {
        printf("[tid: %d] Erro ao enviar resposta inicial de transferencia ao backup.\n", tid);
        return 1;
    }

    // Envia a lista de conexões ao backup conectado
    pthread_mutex_lock(activeConnections.lock);
    send_active_connections_list_all(activeConnections);
    pthread_mutex_unlock(activeConnections.lock);
    return 0;
}

// Função para receber conexões de servidores de backup 
int connectBackup(Connection_t server, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer)
{
    deviceID = package.package_specific.replicaManagerIdentification.deviceID;

    std::optional<DeviceConnectReturn> deviceConnectReturn;

    // Tenta conectar com o replica manager
    std::string username = "backup";
    deviceConnectReturn = deviceManager.connect(server, username, false);

    if (!deviceConnectReturn.has_value()) {
        printf("[tid: %d] Nova conexao do backup rejeitada.\n", tid);

        // Responde ao usuário indicando rejeição da conexão
        package = Package(PackageReplicaManagerIdentificationResponse(REJECTED_RM, 0));
        write_package_to_socket(server.socket_id, package, fileContentBuffer);
        return 1;
    }

    deviceID = deviceConnectReturn.value().deviceID;

    printf("[tid: %d] Nova conexao com backup, dispositivo: 0x%02x.\n", tid, (uint8_t)deviceID);

    // Responde ao backup indicando sucesso da conexão
    package = Package(PackageReplicaManagerIdentificationResponse(ACCEPTED_RM, deviceID));

    if (write_package_to_socket(server.socket_id, package, fileContentBuffer))
    {
        printf("[tid: %d] Erro ao enviar resposta inicial ao backup.\n", tid);
        return 1;
    }

    return 0;
}

// Processa pacotes iniciais de identificação
int connectUser(Connection_t client, std::string &username, User *&user, Device *&device, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer)
{
    username = std::string(package.package_specific.userIdentification.user_name);
    deviceID = package.package_specific.userIdentification.deviceID;

    std::optional<DeviceConnectReturn> deviceConnectReturn;

    // Tenta conectar como um dispositivo do usuário
    deviceConnectReturn = deviceManager.connect(client, username, false);

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

    // Envia a lista de conexões aos backups conectados
    pthread_mutex_lock(activeConnections.lock);
    send_active_connections_list_all(activeConnections);
    pthread_mutex_unlock(activeConnections.lock);

    return 0;
}

void *serverThread(void *arg)
{
    client.socket_id = ((ServerThreadArg*)arg)->socket_id;
    client.host      = ((ServerThreadArg*)arg)->host;
    client.port      = ((ServerThreadArg*)arg)->port;

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
    if (connectToServer(client, username, user, device, deviceID))
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
