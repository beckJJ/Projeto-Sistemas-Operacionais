#include "backupThread.hpp"
#include "deviceManager.hpp"
#include "connections.hpp"
#include "serverThread.hpp"
#include "replicaManager.hpp"

#include "../Common/DadosConexao.hpp"
#include "../Common/package.hpp"
#include "../Common/package_file.hpp"

#include <pthread.h>
#include <threads.h>
#include <string.h>

extern DeviceManager deviceManager;
extern ActiveConnections_t activeConnections;

//thread_local pid_t tid = 0;
thread_local Connection_t mainServer = {{0}, {0}, -1};


// Thread que fica recebendo novas conexões do servidor principal
void *backupThread(void *arg)
{
    DadosConexao dadosConexao = DadosConexao();
    strcpy(dadosConexao.endereco_ip, ((ServerThreadArg*)arg)->endereco_ip);
    strcpy(dadosConexao.numero_porta, ((ServerThreadArg*)arg)->numero_porta);

    if (conecta_backup_main(dadosConexao)) {
        exit(EXIT_FAILURE);
    } else {
        printf("Thread secundaria conectada\n");
    }

    while (true) {
        std::vector<char> fileContentBuffer;
        Package package;

        if (read_package_from_socket(dadosConexao.socket, package, fileContentBuffer)) {
            printf("Erro ao ler pacote do servidor.\n");    pthread_mutex_lock(activeConnections.lock);
            break;
        } else {
            pthread_mutex_lock(activeConnections.lock);
            printf("Clientes conectados:\n");
            for (Connection_t c : activeConnections.clients) {
                printf("%s\t", c.endereco_ip);
                printf("%s\t", c.numero_porta);
                printf("%d\n", c.socket_id);
            }
            printf("Backups conectados:\n");
            for (Connection_t c : activeConnections.backups) {
                printf("%s\t", c.endereco_ip);
                printf("%s\t", c.numero_porta);
                printf("%d\n", c.socket_id);
            }
            printf("\n");
            pthread_mutex_unlock(activeConnections.lock);
        }
    }
    return NULL;
}