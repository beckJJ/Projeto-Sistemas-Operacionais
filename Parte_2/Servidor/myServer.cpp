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
#include "pingThread.hpp"

#if DEBUG_PACOTE
// Lock usado por print_package para exibir completamente um pacote
pthread_mutex_t print_package_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

// Lock utilizado para um backup se conectar com uma thread de cada vez
pthread_mutex_t backup_connection_lock = PTHREAD_MUTEX_INITIALIZER;

DeviceManager deviceManager = DeviceManager();
ActiveConnections_t activeConnections;
DadosConexao dadosConexao = DadosConexao();

void cancel_threads()
{
    if (dadosConexao.backup_thread.has_value()) {
        pthread_cancel(dadosConexao.backup_thread.value());
    }
    if (dadosConexao.ping_thread.has_value()) {
        pthread_cancel(dadosConexao.ping_thread.value());
    }
}

void close_sockets()
{
    if (dadosConexao.socket_ping != -1) {
        close(dadosConexao.socket_ping);
    }
    if (dadosConexao.socket_transfer != -1) {
        close(dadosConexao.socket_transfer);
    }
}

void sigint_handler_main(int)
{
    printf("Main thread received SIGINT.\n");

    // Desconecta todos os dispositivos
    deviceManager.disconnect_all();
    // Cancela os threads
    cancel_threads();
    // Fecha os sockets
    close_sockets();
    if (dadosConexao.socket != -1) {
        close(dadosConexao.socket);
    }
    // Encerra o servidor
    exit(EXIT_FAILURE);
}

void bind_socket()
{
    struct sockaddr_in serv_addr;

    if ((dadosConexao.socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Erro! Nao foi possivel iniciar utilizacao do socket do servidor!\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(dadosConexao.listen_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), sizeof(serv_addr.sin_zero));

    if (bind(dadosConexao.socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Erro! Nao foi possivel atribuir uma identidade ao socket do servidor!\n");
        exit(EXIT_FAILURE);
    }

    listen(dadosConexao.socket, 5);
}

int main(int argc, char *argv[])
{
    dadosConexao.listen_port = PORT;
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
            dadosConexao.listen_port = atoi(optarg);
            break;
        case 'b':  // inicializar server como backup
            dadosConexao.backup_flag = true;
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

    if (dadosConexao.backup_flag) {
        printf("Inicializando Backup\n");
        if (!dadosConexao.endereco_ip[0]) {
            printf("Erro: Informe o endereco ip do servidor principal\n");
            exit(0);
        }
        if (!dadosConexao.numero_porta[0]) {
            printf("Erro: informe a porta do servidor principal\n");
            exit(0);
        }
    }

    // Cria sync_dir do servidor
    if (create_dir_if_not_exists(PREFIXO_DIRETORIO_SERVIDOR))
    {
        std::cout << "Nao foi possivel criar diretorio " << PREFIXO_DIRETORIO_SERVIDOR << std::endl;
        return 1;
    }

    bind_socket();

    printf("Servidor está escutando na porta: %d.\n", dadosConexao.listen_port);

    if (dadosConexao.backup_flag) {
        printf("Conectando no servidor principal %s:%s\n", dadosConexao.endereco_ip, dadosConexao.numero_porta);
        // Inicia thread para ficar aguardando novas conexões no servidor principal 
        pthread_t backup_thread;
        pthread_create(&backup_thread, NULL, backupThread, NULL);
        dadosConexao.backup_thread = backup_thread;

        // Iniciar thread de ping 
        pthread_t ping_thread;
        pthread_create(&ping_thread, NULL, pingThread, NULL);
        dadosConexao.ping_thread = ping_thread;
        
        // Fica em busy waiting até deixar de ser backup
        while (dadosConexao.backup_flag) {
            socklen_t backup_len;
            struct sockaddr_in backup_addr;
            backup_len = sizeof(struct sockaddr_in);

            int socket_id = -1;
            // Aguardando pacotes de election...
            if ((socket_id = accept(dadosConexao.socket, (struct sockaddr *)&backup_addr, &backup_len)) == -1) {
                continue;
            }
            if (!dadosConexao.backup_flag) {
                break;
            }

            Package package = Package();
            std::vector<char> fileContentBuffer;
            pthread_mutex_lock(dadosConexao.socket_lock);
            if (read_package_from_socket(socket_id, package, fileContentBuffer)) {
                printf("Nenhum pacote recebido\n");
            } else {
                printf("Pacote recebido\n");
            }
            pthread_mutex_unlock(dadosConexao.socket_lock);
            
            switch(package.package_type) {
            case REPLICA_MANAGER_ELECTION_ELECTION: // se recebeu election
                // envia um answer
                printf("Election Recebido\n");
                pthread_mutex_lock(dadosConexao.socket_lock);
                send_answer_to_socket(socket_id);
                pthread_mutex_unlock(dadosConexao.socket_lock);
                printf("Answer enviado\n");
                break;
            case REPLICA_MANAGER_ELECTION_COORDINATOR: // se recebeu coordinator
                printf("Coordinator recebido, id = %d\n", package.package_specific.replicaManagerElectionCoordinator.deviceID);
                // mata threads e fecha sockets abertos com o servidor antigo
                cancel_threads();
                close_sockets();
                // TODO: busca id do coordenador na lista de backups e salva em um temp

                // limpa as listas de clientes e backups conectados
                pthread_mutex_lock(activeConnections.lock);
                activeConnections.clients.clear();
                activeConnections.backups.clear();
                pthread_mutex_unlock(activeConnections.lock);
                // TODO: cria threads com o novo coordenador

                break;
            default:
                break;
            }
            close(socket_id);
        }
        printf("Saiu do while, backup eleito\n");
        // cancelar threads abertas e fechar sockets 
        cancel_threads();
        close_sockets();
        // salvar clients e backups em listas temporárias
        pthread_mutex_lock(activeConnections.lock);
        std::vector<Connection_t> temp_backups = activeConnections.backups;
        std::vector<Connection_t> temp_clients = activeConnections.clients;
        // remove todas conexões ativas de cliente e servidor
        activeConnections.clients.clear();
        activeConnections.backups.clear();
        pthread_mutex_unlock(activeConnections.lock);
        // enviar coordinator para os outros backups
        for (Connection_t backup : temp_backups) {
            if (backup.socket_id != dadosConexao.deviceID_transfer) {
                printf("Enviando coordinator...\n");
                DadosConexao dadosConexao_backup = DadosConexao();
                char *endereco_ip = inet_ntoa(*(struct in_addr *)&backup.host);
                strcpy(dadosConexao_backup.endereco_ip, endereco_ip);
                sprintf(dadosConexao_backup.numero_porta, "%d", backup.port);
                std::optional<int> socket_opt = conecta_servidor(dadosConexao_backup);
                if (!socket_opt.has_value()) {
                    printf("Erro na abertura de conexao\n");
                    continue;
                }
                // enviar pacote coordinator
                int current_socket = socket_opt.value();
                printf("Enviando coordinator\n");
                pthread_mutex_lock(dadosConexao.socket_lock);
                send_coordinator_to_socket(current_socket, dadosConexao.deviceID_transfer);
                pthread_mutex_unlock(dadosConexao.socket_lock);
            }
        }

        // TODO: realizar nova conexão com os clientes

    }

    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(struct sockaddr_in);

    while (true)
    {
        ServerThreadArg thread_arg; // { };
        pthread_t thread;

        printf("Aguardando nova conexao...\n");
        if ((thread_arg.socket_id = accept(dadosConexao.socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
            printf("Erro! Nao foi possivel realizar conexao com o cliente!\n");
            break;
        }

        thread_arg.host = *(uint32_t*)&cli_addr.sin_addr;

        printf("Nova conexao estabelecida.\n");

        // Cria thread para lidar com a conexão estabelecida    
        pthread_create(&thread, NULL, serverThread, &thread_arg);
    }

    close(dadosConexao.socket);

    return 0;
}
