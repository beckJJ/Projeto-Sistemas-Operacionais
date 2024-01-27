#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <vector>
#include <arpa/inet.h>

#include "serverThread.hpp"
#include "../Common/functions.hpp"
#include "config.hpp"
#include <signal.h>
#include "../Common/connections.hpp"
#include "../Common/DadosConexao.hpp"
#include "replicaManager.hpp"
#include "../Common/package_file.hpp"
#include "backupThread.hpp"

#if DEBUG_PACOTE
// Lock usado por print_package para exibir completamente um pacote
pthread_mutex_t print_package_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

DeviceManager deviceManager = DeviceManager();
int main_thread_socket = -1;
ActiveConnections_t activeConnections;
bool backup = false;
DadosConexao dadosConexao = DadosConexao();

void sigint_handler_main(int)
{
    printf("Main thread received SIGINT.\n");

    // Desconecta todos os dispositivos
    deviceManager.disconnect_all();

    if (main_thread_socket != -1)
    {
        close(main_thread_socket);
    }

    // Encerra o servidor
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    uint16_t port = PORT;
    char c;

    activeConnections.lock = new pthread_mutex_t;

    // Lê argumentos do programa
    while ((c = getopt(argc, argv, "bhp:s:m:")) != -1)
    {
        switch (c)
        {
        case 'h':
            puts("Opcoes:");
            puts("\t-p PORT\tPorta que devera ser usada pelo servidor.");
            puts("\t-h\tExibe mensagem de ajuda.");
            exit(0);
        case 'p':
            port = atoi(optarg);
            break;
        case 'b':  // inicializar server como backup
            backup = true;
            break;
        case 's': // informar ip do servidor principal
            strcpy(dadosConexao.endereco_ip, optarg);
            break;
        case 'm': // informar a porta do servidor principal
            strcpy(dadosConexao.numero_porta, optarg);
            break;
        default:
            printf("Usage:\n");
            printf("\t%s [-p PORT] [-b] [-s SERVER_IP] [-m SERVER_PORT]\n", argv[0]);
            abort();
        }
    }

    // Registra sigint_handler_main para SIGINT
    signal(SIGINT, sigint_handler_main);

    if (backup) {
        printf("Inicializando Backup\n");
        if (!dadosConexao.endereco_ip[0]) {
            printf("Erro: Informe o endereco ip do servidor principal\n");
            exit(0);
        }
        if (!dadosConexao.numero_porta[0]) {
            printf("Erro: informe a porta do servidor principal\n");
            exit(0);
        }
//        printf("Conectando no servidor principal %s:%s\n", dadosConexao.endereco_ip, dadosConexao.numero_porta);

        // Iniciar thread de ping 
        ServerThreadArg ping_thread_arg;
        pthread_t ping_thread;
        ping_thread_arg.port = atoi(dadosConexao.numero_porta);
        strcpy(ping_thread_arg.hostname, dadosConexao.endereco_ip);

        pthread_create(&ping_thread, NULL, pingThread, &ping_thread_arg);
    }

    // Cria sync_dir do servidor
    if (create_dir_if_not_exists(PREFIXO_DIRETORIO_SERVIDOR))
    {
        std::cout << "Nao foi possivel criar diretorio " << PREFIXO_DIRETORIO_SERVIDOR << std::endl;
        return 1;
    }

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if ((main_thread_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Erro! Nao foi possivel iniciar utilizacao do socket do servidor!\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), sizeof(serv_addr.sin_zero));

    if (bind(main_thread_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Erro! Nao foi possivel atribuir uma identidade ao socket do servidor!\n");
        exit(EXIT_FAILURE);
    }

    listen(main_thread_socket, 5);

    printf("Servidor está escutando na porta: %d.\n", port);

    clilen = sizeof(struct sockaddr_in);

    while (1)
    {
        if (backup) {
            ServerThreadArg backup_thread_arg;

            backup_thread_arg.port = atoi(dadosConexao.numero_porta);
            strcpy(backup_thread_arg.hostname, dadosConexao.endereco_ip);

            pthread_t backup_thread;
            
            // Inicia thread para ficar aguardando novas conexões no servidor principal 
            pthread_create(&backup_thread, NULL, backupThread, &backup_thread_arg);

            // Thread atual fica enviando pings para o servidor principal e recebendo ACKs
            while (true) {
 /*               send_ping_to_main(dadosConexao.socket);
                std::vector<char> fileContentBuffer;
                Package package;
                if (read_package_from_socket(dadosConexao.socket, package, fileContentBuffer)) {
                    printf("Erro ao ler pacote do servidor.\n");
                    exit(0);
                    // algoritmo de seleção 
                    // setar backup = false para o escolhido
                }
                // Resposta inválida
                if (package.package_type != REPLICA_MANAGER_PING_RESPONSE) {
                    printf("Resposta invalida do servidor.\n");
                    exit(0);
                }
            //    printf("ACK RECEBIDO\n"); */
                sleep(1);
            }
        } else {
            ServerThreadArg thread_arg; // { };
            pthread_t thread;

            printf("Aguardando nova conexao...\n");
            if ((thread_arg.socket_id = accept(main_thread_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
                printf("Erro! Nao foi possivel realizar conexao com o cliente!\n");
                break;
            }

            thread_arg.host = *(uint32_t*)&cli_addr.sin_addr;

            printf("Nova conexao estabelecida.\n");

            // Cria thread para lidar com a conexão estabelecida    
            pthread_create(&thread, NULL, serverThread, &thread_arg);
            
        }
    }

    close(main_thread_socket);

    return 0;
}
