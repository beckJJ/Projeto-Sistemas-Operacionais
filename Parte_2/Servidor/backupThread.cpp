#include "backupThread.hpp"
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

//thread_local pid_t tid = 0;
thread_local Connection_t client_backup = Connection_t(0, 0, 0xFFFF, "");

void create_client_dirs(std::vector<Connection_t> clientList, const std::string base_path)
{
    for (Connection_t c : clientList) {
        std::string path = base_path;
        path.append(c.user_name);
        create_dir_if_not_exists(path.c_str());
    }
}

void handleChangeEvent(PackageChangeEvent &changeEvent, std::string &path_base, int socket_id)
{
    std::vector<char> fileContentBuffer;
    std::string old_path = path_base;
    std::string new_path = path_base;
    std::string path;
    FILE *fp;

    switch (changeEvent.event) {
    // Remove arquivo de sync dir local
    case FILE_DELETED:
        path = path_base;
        path.append(changeEvent.filename1);
        if (remove(path.c_str())) {
            printf("Erro ao remover arquivo \"%s\".\n", changeEvent.filename1);
        }
        break;
    // Cria arquivo no sync dir local
    case FILE_CREATED:
        path = path_base;
        path.append(changeEvent.username);
        path.append("/");
        path.append(changeEvent.filename1);
        fp = fopen(path.c_str(), "w");

        if (!fp) {
            printf("Erro ao criar arquivo \"%s\".\n", changeEvent.filename1);
        } else {
            fclose(fp);
        }
        break;
    // Baixa versão atualizada do arquivo no sync dir local
    case FILE_MODIFIED:
        path = path_base;
        path.append(changeEvent.username);
        path.append("/");
        path.append(changeEvent.filename1);

        // Após o evento FILE_MODIFIED haverá pacotes de upload de arquivo
        if (read_upload_file_and_save(socket_id, path.c_str())) {
            printf("Nao foi possivel baixar versao modificada do arquivo \"%s\".\n", changeEvent.filename1);
        }
        break;
    // Renomeia arquivo no sync dir local
    case FILE_RENAME:
        old_path = path_base;
        old_path.append(changeEvent.username);
        old_path.append("/");
        old_path.append(changeEvent.filename1);

        new_path = path_base;
        new_path.append(changeEvent.username);
        new_path.append("/");
        new_path.append(changeEvent.filename2);

        if (rename(old_path.c_str(), new_path.c_str())) {
            printf("Erro ao renomear arquivo de \"%s\" para \"%s\".\n", changeEvent.filename1, changeEvent.filename2);
        }
        break;
    default:
        printf("Evento desconhecido: 0x%02x\n", (uint8_t)changeEvent.event);
        break;
    }
}

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
    std::string path_base = std::string(PREFIXO_DIRETORIO_SERVIDOR);
    path_base.append("/");
    uint16_t listen_port = ((ServerThreadArg*)arg)->listen_port;

    if (conecta_backup_transfer_main(dadosConexao, listen_port)) {
        exit(EXIT_FAILURE);
    } else {
        printf("Thread de transferencia conectada\n");
    }

    while (true) {
        std::vector<char> fileContentBuffer;
        Package package;

        if (read_package_from_socket(dadosConexao.socket, package, fileContentBuffer)) {
            printf("Erro ao ler pacote do servidor.\n");
            break;
        }

        printf("\nRECEBIDO PACOTE: %d\n",package.package_type);
        switch(package.package_type) {
            case ACTIVE_CONNECTIONS_LIST:
                printf("\nRECEBIDO NOVO PACOTE DE ACTIVE_CONNECTIONS_LIST\n");
                // Ler sequencia de conexoes ativas
                pthread_mutex_lock(activeConnections.lock);
                handleActiveConnectionsList(package, dadosConexao.socket);
                pthread_mutex_unlock(activeConnections.lock);
                // Criar diretorios para cada um dos usuarios ativos em seu sync dir
                pthread_mutex_lock(activeConnections.lock);
                create_client_dirs(activeConnections.clients, path_base);
                pthread_mutex_unlock(activeConnections.lock);

                pthread_mutex_lock(activeConnections.lock);
                printf("Clientes conectados:\n");
                for (Connection_t c : activeConnections.clients) {
                    char *endereco_ip = inet_ntoa(*(struct in_addr *)&c.host);
                    printf("%s\t", endereco_ip);
                    printf("%d\t", c.port);
                    printf("%d\t", c.socket_id);
                    printf("%s\n", c.user_name);
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
                break;
            case CHANGE_EVENT:
                printf("\nRECEBIDO NOVO PACOTE DE CHANGE_EVENT\n");
                handleChangeEvent(package.package_specific.changeEvent, path_base, dadosConexao.socket);
                break;
            default:
                break;
        }    
    }
    return NULL;
}