#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <vector>
#include <arpa/inet.h>

#include "serverThread.hpp"
#include "../Common/functions.hpp"
#include "config.hpp"
#include <signal.h>
#include "connections.hpp"

#if DEBUG_PACOTE
// Lock usado por print_package para exibir completamente um pacote
pthread_mutex_t print_package_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

DeviceManager deviceManager = DeviceManager();
int main_thread_socket = -1;
ActiveConnections_t activeConnections;

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

    // Lê argumentos do programa
    while ((c = getopt(argc, argv, "hp:")) != -1)
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
        default:
            printf("Usage:\n");
            printf("\t%s [-p PORT]\n", argv[0]);
            abort();
        }
    }

    // Registra sigint_handler_main para SIGINT
    signal(SIGINT, sigint_handler_main);

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

    activeConnections.lock = new pthread_mutex_t;

    while (1)
    {
        ServerThreadArg thread_arg; // { };
        pthread_t thread;

        if ((thread_arg.socket_id = accept(main_thread_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1)
        {
            printf("Erro! Nao foi possivel realizar conexao com o cliente!\n");
            break;
        }
        thread_arg.socket_address = cli_addr;

        printf("Nova conexao estabelecida.\n");
/*
        printf("Clientes conectados:\n");
        for (sockaddr_in c : activeConnections.clients) {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(c.sin_addr), clientIP, INET_ADDRSTRLEN);
            printf("%s\t", clientIP);
            printf("%d\n", ntohs(c.sin_port));

        }
        printf("\n");
*/
        // Cria thread para lidar com a conexão estabelecida
        pthread_create(&thread, NULL, serverThread, &thread_arg);
    }

    close(main_thread_socket);

    return 0;
}
