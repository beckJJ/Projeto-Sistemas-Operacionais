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

void break_accept_on_main_thread()
{
    sleep(5);
    // seta backup = false porque foi escolhido
    dadosConexao.backup_flag = false;
    printf("Quebrando accept do main...\n");
    DadosConexao dadosConexao_main = DadosConexao();
    strcpy(dadosConexao_main.endereco_ip, "127.0.0.1");
    sprintf(dadosConexao_main.numero_porta, "%d", dadosConexao.listen_port);
    conecta_servidor(dadosConexao_main);
}

int send_election_to_backups_list(std::vector<Connection_t> backups)
{
    int answers_received = 0;
    for (Connection_t c : backups) {
        if (c.socket_id > dadosConexao.deviceID_transfer) {
            printf("Enviando election\n");
            // Abrir conexao
            DadosConexao dadosConexao_backup = DadosConexao();
            char *endereco_ip = inet_ntoa(*(struct in_addr *)&c.host);
            strcpy(dadosConexao_backup.endereco_ip, endereco_ip);
            sprintf(dadosConexao_backup.numero_porta, "%d", c.port);
            std::optional<int> socket_opt = conecta_servidor(dadosConexao_backup);
            if (!socket_opt.has_value()) {
                printf("Erro na abertura de conexao\n");
                continue;
            }
            // enviar pacote election
            int current_socket = socket_opt.value();
            printf("Enviando election\n");
            pthread_mutex_lock(dadosConexao.socket_lock);
            send_election_to_socket(current_socket);
            pthread_mutex_unlock(dadosConexao.socket_lock);
            printf("Election enviado!\n");
            // receber answer
            Package package = Package();
            std::vector<char> fileContentBuffer;
            pthread_mutex_lock(dadosConexao.socket_lock);
            if (read_package_from_socket(current_socket, package, fileContentBuffer)) {
                pthread_mutex_unlock(dadosConexao.socket_lock);
                printf("Erro na leitura de answer");
                continue;
            }
            pthread_mutex_unlock(dadosConexao.socket_lock);
            // se recebeu um answer, incrementa
            if (package.package_type == REPLICA_MANAGER_ELECTION_ANSWER) {
                answers_received++;
            }
        }
    }
    return answers_received;
}

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
        send_ping_to_socket(dadosConexao.socket_ping);
        std::vector<char> fileContentBuffer;
        Package package;
        if (read_package_from_socket(dadosConexao.socket_ping, package, fileContentBuffer)) {
            printf("Erro ao ler pacote do servidor.\n");
            printf("Fechando sockets abertos...\n");
            close(dadosConexao.socket_ping);
            close(dadosConexao.socket_transfer);
            // inicia algoritmo de seleção
            pthread_mutex_lock(activeConnections.lock);
            int answers_received = send_election_to_backups_list(activeConnections.backups);
            pthread_mutex_unlock(activeConnections.lock);
            
            // se não recebeu nenhum answer, foi eleito 
            if (answers_received == 0) {
                break_accept_on_main_thread();
            }
            // else : aguarda coordinator e inicia novas conexões
            while(true);
        }
        // Resposta inválida
        if (package.package_type != REPLICA_MANAGER_PING_RESPONSE) {
            printf("Resposta invalida do servidor.\n");
            exit(0);
        }
        sleep(1);
    }
}