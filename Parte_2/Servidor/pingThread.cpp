#include "pingThread.hpp"

#include "deviceManager.hpp"
#include "serverThread.hpp"
#include "replicaManager.hpp"

#include "../Common/DadosConexao.hpp"
#include "../Common/package.hpp"
#include "../Common/package_file.hpp"
#include "../Common/connections.hpp"
#include "../Common/functions.hpp"

#include <pthread.h>
#include <threads.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

extern DeviceManager deviceManager;
extern ActiveConnections_t activeConnections;
extern DadosConexao dadosConexao;

void *pingThread(void *)
{
    pthread_mutex_lock(dadosConexao.backup_connection_lock);
    if (conecta_backup_main(dadosConexao)) {
        pthread_mutex_unlock(dadosConexao.backup_connection_lock);
        exit(EXIT_FAILURE);
    } else {
        printf("Thread de ping conectada\n");
    }
    pthread_mutex_unlock(dadosConexao.backup_connection_lock);

    while (true) {
        send_ping_to_main(dadosConexao.socket_ping);
        std::vector<char> fileContentBuffer;
        Package package;
        if (read_package_from_socket(dadosConexao.socket_ping, package, fileContentBuffer)) {
            printf("Erro ao ler pacote do servidor.\n");
            close(dadosConexao.socket_ping);
            // inicia algoritmo de seleção
            pthread_mutex_lock(activeConnections.lock);
     //       int answers_received = send_election_to_backups_list(activeConnections.backups);
            int answers_received = 0;
            pthread_mutex_unlock(activeConnections.lock);
            
            // se não recebeu nenhum answer, foi eleito 
            if (answers_received == 0) {
                // seta backup = false se for escolhido
                dadosConexao.backup_flag = false;
         //       break;
         //       break_accept_on_main_thread();
            }
            // else : aguarda coordinator e inicia novas conexões
            while(dadosConexao.backup_flag);
        }
        // Resposta inválida
        if (package.package_type != REPLICA_MANAGER_PING_RESPONSE) {
            printf("Resposta invalida do servidor.\n");
            exit(0);
        }
        sleep(1);
    }
}