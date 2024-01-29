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
extern DadosConexao dadosConexao;

thread_local Connection_t client_backup = Connection_t(0, 0, 0xFFFF, "");

// Encerra thread
void exitBackupThread(void)
{
    dadosConexao.backup_thread = std::nullopt;
    pthread_exit(NULL);
}

void create_client_dirs(std::vector<Connection_t> clientList, const std::string base_path)
{
    for (Connection_t c : clientList) {
        std::string path = base_path;
        path.append(c.user_name);
        create_dir_if_not_exists(path.c_str());
    }
}

void handleCommitEvent(PackageChangeEvent &prevPackage, std::string &path_base)
{
    std::vector<char> fileContentBuffer;
    std::string old_path = path_base;
    std::string new_path = path_base;
    std::string path;

    switch (prevPackage.event) {
    case FILE_DELETED:
        path = path_base;
        path.append(prevPackage.username);
        path.append("/");
        path.append(prevPackage.filename1);
        path.append(".tmp");
        if (remove(path.c_str())) {
            printf("Erro ao remover arquivo \"%s\".\n", prevPackage.filename1);
            return;
        }
        break;
    case FILE_CREATED:
        new_path = path_base;
        new_path.append(prevPackage.username);
        new_path.append("/");
        new_path.append(prevPackage.filename1);
        old_path = new_path;
        old_path.append(".tmp");
        if (rename(old_path.c_str(), new_path.c_str())) {
            printf("Erro ao criar arquivo \"%s\".\n", prevPackage.filename1);
            return;
        }
        break;
    case FILE_MODIFIED:
        // remove versao antiga (.tmp)
        path = path_base;
        path.append(prevPackage.username);
        path.append("/");
        path.append(prevPackage.filename1);
        path.append(".tmp");
        if (remove(path.c_str())) {
            printf("Nao foi possivel baixar versao modificada do arquivo \"%s\".\n", prevPackage.filename1);
            return;
        }
        break;
    case FILE_RENAME:
        old_path = path_base;
        old_path.append(prevPackage.username);
        old_path.append("/");
        old_path.append(prevPackage.filename1);
        old_path.append(".tmp");
        new_path = path_base;
        new_path.append(prevPackage.username);
        new_path.append("/");
        new_path.append(prevPackage.filename2);
        if (rename(old_path.c_str(), new_path.c_str())) {
            printf("Erro ao renomear arquivo de \"%s\" para \"%s\".\n", prevPackage.filename1, prevPackage.filename2);
            return;
        }
        break;
    default:
        printf("Evento desconhecido: 0x%02x\n", (uint8_t)prevPackage.event);
        break;
    }
}

int handleChangeEvent(PackageChangeEvent &changeEvent, std::string &path_base, int socket_id)
{
    std::vector<char> fileContentBuffer;
    std::string old_path = path_base;
    std::string new_path = path_base;
    std::string path;
    FILE *fp;

    switch (changeEvent.event) {
    // Remove arquivo de sync dir local
    case FILE_DELETED:
        // ao inves de remover arquivo, apenas renomear para [filename].tmp
        old_path = path_base;
        old_path.append(changeEvent.username);
        old_path.append("/");
        old_path.append(changeEvent.filename1);

        new_path = old_path;
        new_path.append(".tmp");

        if (rename(old_path.c_str(), new_path.c_str())) {
            printf("Erro ao remover arquivo \"%s\".\n", changeEvent.filename1);
            return -1;
        }
        break;
    // Cria arquivo no sync dir local
    case FILE_CREATED:
        // criar arquivo com [filename].tmp
        path = path_base;
        path.append(changeEvent.username);
        path.append("/");
        path.append(changeEvent.filename1);
        path.append(".tmp");
        fp = fopen(path.c_str(), "w");

        if (!fp) {
            printf("Erro ao criar arquivo \"%s\".\n", changeEvent.filename1);
            return -1;
        } else {
            fclose(fp);
        }
        break;
    // Baixa versão atualizada do arquivo no sync dir local
    case FILE_MODIFIED:
        // renomeia versao antiga para [filename].tmp
        path = path_base;
        path.append(changeEvent.username);
        path.append("/");
        path.append(changeEvent.filename1);

        new_path = path;
        new_path.append(".tmp");
        if (rename(path.c_str(), new_path.c_str())) {
            printf("Nao foi possivel renomear arquivo \"%s\", se o arquivo nao existia pode ser ignorado.\n", changeEvent.filename1);
        }

        // Após o evento FILE_MODIFIED haverá pacotes de upload de arquivo
        if (read_upload_file_and_save(socket_id, path.c_str())) {
            printf("Nao foi possivel baixar versao modificada do arquivo \"%s\".\n", changeEvent.filename1);
            return -1;
        }
        break;
    // Renomeia arquivo no sync dir local
    case FILE_RENAME:
        old_path = path_base;
        old_path.append(changeEvent.username);
        old_path.append("/");
        old_path.append(changeEvent.filename1);

        new_path = old_path;
        new_path.append(".tmp");

        // renomeia o arquivo para [nome antigo].tmp
        if (rename(old_path.c_str(), new_path.c_str())) {
            printf("Erro ao renomear arquivo de \"%s\" para \"%s\".\n", changeEvent.filename1, changeEvent.filename2);
            return -1;
        }
        break;
    default:
        printf("Evento desconhecido: 0x%02x\n", (uint8_t)changeEvent.event);
        return -1;
        break;
    }
    return 0;
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
void *backupThread(void *)
{
    std::string path_base = std::string(PREFIXO_DIRETORIO_SERVIDOR);
    path_base.append("/");

    pthread_mutex_lock(dadosConexao.backup_connection_lock);
    if (conecta_backup_transfer_main(dadosConexao)) {
        pthread_mutex_unlock(dadosConexao.backup_connection_lock);
        exitBackupThread();
    } else {
        printf("Thread de transferencia conectada\n");
    }
    pthread_mutex_unlock(dadosConexao.backup_connection_lock);

    while (true) {
        std::vector<char> fileContentBuffer;
        Package package;

        Package last_package; // utilizado para saber qual o ultimo evento

        if (read_package_from_socket(dadosConexao.socket_transfer, package, fileContentBuffer)) {
            printf("Erro ao ler pacote do servidor.\n");
            exitBackupThread();
        }

        printf("\nRECEBIDO PACOTE: %d\n",package.package_type);
        switch(package.package_type) {
        case ACTIVE_CONNECTIONS_LIST:
            printf("\nRECEBIDO NOVO PACOTE DE ACTIVE_CONNECTIONS_LIST\n");
            // Ler sequencia de conexoes ativas
            pthread_mutex_lock(activeConnections.lock);
            handleActiveConnectionsList(package, dadosConexao.socket_transfer);
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
            // salva o pacote
            last_package = package;
            // Muda o event em um 
            if (handleChangeEvent(package.package_specific.changeEvent, path_base, dadosConexao.socket_transfer)) {
                // ocorreu um erro ao modificar o arquivo
            } else {
                // envia um OK pro servidor principal
                pthread_mutex_lock(dadosConexao.socket_lock);
                send_transaction_ok_to_socket(dadosConexao.socket_transfer);
                pthread_mutex_unlock(dadosConexao.socket_lock);
            }
            break;
        case COMMIT_EVENT:
            printf("\nRECEBIDO NOVO PACOTE DE COMMIT_EVENT\n");
            handleCommitEvent(last_package.package_specific.changeEvent, path_base);
            break;
        default:
            break;
        }    
    }
    exitBackupThread();
}