#include "frontendThread.hpp"

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
#include <signal.h>

#include "../Common/package.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/DadosConexao.hpp"
#include "../Common/defines.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_file.hpp"
#include "readThread.hpp"
#include "auxiliaresCliente.hpp"

extern DadosConexao dados_conexao;

void exitFrontendThread(void)
{
    dados_conexao.frontend_thread = std::nullopt;
    pthread_exit(NULL);
}

void *frontendThread(void *)
{
    // bind socket do frontend
    struct sockaddr_in frontend_addr;
    if ((dados_conexao.socket_frontend = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Erro! Nao foi possivel iniciar utilizacao do socket do frontend!\n");
        exitFrontendThread();
    }

    frontend_addr.sin_family = AF_INET;
    frontend_addr.sin_port = htons(dados_conexao.listen_port);
    frontend_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(frontend_addr.sin_zero), sizeof(frontend_addr.sin_zero));

    if (bind(dados_conexao.socket_frontend, (struct sockaddr *)&frontend_addr, sizeof(frontend_addr)) < 0) {
        printf("Erro! Nao foi possivel atribuir uma identidade ao socket do frontend!\n");
        exitFrontendThread();
    }
 
    listen(dados_conexao.socket_frontend, 5);

    while(true) {
        // aguardando pacotes de novas conexoes dos backups
        socklen_t backup_len;
        struct sockaddr_in backup_addr;
        backup_len = sizeof(struct sockaddr_in);
        int socket_id = -1;
      //  printf("Aguardando pacotes de novas conexoes...\n");
        if ((socket_id = accept(dados_conexao.socket_frontend, (struct sockaddr *)&backup_addr, &backup_len)) == -1) {
            continue;
        }
        std::vector<char> fileContentBuffer;
        Package package = Package();
        if (read_package_from_socket(socket_id, package, fileContentBuffer)) {
            printf("Nenhum pacote recebido\n");
        }

        switch (package.package_type) {
            case NEW_SERVER_INFO: {
                // imprimir dados do servidor
                char *endereco_ip = inet_ntoa(*(struct in_addr *)&package.package_specific.newServerInfo.host);
                printf("Endereco do novo servidor: %s:%d\n", endereco_ip, package.package_specific.newServerInfo.port);
                
                printf("Conectando-se ao servidor...\n");
                sleep(5);
                // TODO: apontar novas conexoes para o servidor recebido
                strcpy(dados_conexao.endereco_ip, endereco_ip);
                sprintf(dados_conexao.numero_porta, "%d", package.package_specific.newServerInfo.port);
                if (conecta_device(dados_conexao)) {
                    continue;
                }
                // Iniciar nova thread de read
                pthread_t new_thread;
                pthread_create(&new_thread, NULL, readThread, NULL);
                dados_conexao.sync_thread = new_thread;
                printf("Conectado ao novo servidor!\n");
                break;
            }
            default:
                printf("Package type: 0x%2x\n", package.package_type);
                break;
        }
        close(socket_id);
    }  
}