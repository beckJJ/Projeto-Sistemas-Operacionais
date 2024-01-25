#include "backupThread.hpp"
#include "deviceManager.hpp"
#include "serverThread.hpp"
#include "replicaManager.hpp"

#include "../Common/DadosConexao.hpp"
#include "../Common/package.hpp"
#include "../Common/package_file.hpp"
#include "../Common/connections.hpp"

#include <pthread.h>
#include <threads.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

extern DeviceManager deviceManager;
extern ActiveConnections_t activeConnections;

//thread_local pid_t tid = 0;
thread_local Connection_t client_backup = Connection_t(0, 0, 0xFFFF, "");

// Lê uma sequência de pacotes de PackageActiveConnectionsList, package já é um pacote que deve ser processado
void handleActiveConnectionsList(Package &package, int socket_id)
{
    std::vector<char> fileContentBuffer;
    int size_clients = 1, size_backups = 1; // mesmo se a lista estiver vazia, recebe 1 pacote de cada
    int pacotes_recebidos = 0;
    activeConnections.clients.clear();
    activeConnections.backups.clear();

    while (package.package_type == ACTIVE_CONNECTIONS_LIST) {
        if (package.package_specific.activeConnectionsList.is_client) {
            size_clients = package.package_specific.activeConnectionsList.count;
            if (package.package_specific.activeConnectionsList.count == 0) {
                size_clients = 1;
                activeConnections.clients.clear();
            } else {
                activeConnections.clients.push_back(package.package_specific.activeConnectionsList.connection);
            }
        } else {
            size_backups = package.package_specific.activeConnectionsList.count;
            if (package.package_specific.activeConnectionsList.count == 0) {
                size_backups = 1;
                activeConnections.backups.clear();
            } else {
                activeConnections.backups.push_back(package.package_specific.activeConnectionsList.connection);
            }
        }
        pacotes_recebidos++;
        if (pacotes_recebidos >= size_clients + size_backups) {
            return;
        }
        if (read_package_from_socket(socket_id, package, fileContentBuffer)) {
            return;
        }
    }
}

// Thread que fica recebendo novas conexões do servidor principal
void *backupThread(void *arg)
{
    DadosConexao dadosConexao = DadosConexao();
    strcpy(dadosConexao.endereco_ip, ((ServerThreadArg*)arg)->hostname);
    sprintf(dadosConexao.numero_porta, "%d", ((ServerThreadArg*)arg)->port);

    if (conecta_backup_transfer_main(dadosConexao)) {
        exit(EXIT_FAILURE);
    } else {
        printf("Thread secundaria conectada\n");
    }

    while (true) {
        std::vector<char> fileContentBuffer;
        Package package;

        if (read_package_from_socket(dadosConexao.socket, package, fileContentBuffer)) {
            printf("Erro ao ler pacote do servidor.\n");
            break;
        }

        if (package.package_type == ACTIVE_CONNECTIONS_LIST) {
            printf("\nRECEBIDO NOVO PACOTE DE ACTIVE_CONNECTIONS_LIST\n");
            pthread_mutex_lock(activeConnections.lock);
            handleActiveConnectionsList(package, dadosConexao.socket);
            pthread_mutex_unlock(activeConnections.lock);
        }

        pthread_mutex_lock(activeConnections.lock);
        printf("Clientes conectados:\n");
        for (Connection_t c : activeConnections.clients) {
            char *endereco_ip = inet_ntoa(*(struct in_addr *)&c.host);
            printf("%s\t", endereco_ip);
            printf("%d\t", c.port);
            printf("%d\n", c.socket_id);
        }
        printf("Backups conectados:\n");
        for (Connection_t c : activeConnections.backups) {
            char *endereco_ip = inet_ntoa(*(struct in_addr *)&c.host);
            printf("%s\t", endereco_ip);
            printf("%d\t", c.port);
            printf("%d\n", c.socket_id);
        }
        printf("\n");
        pthread_mutex_unlock(activeConnections.lock);
    
    }
    return NULL;
}