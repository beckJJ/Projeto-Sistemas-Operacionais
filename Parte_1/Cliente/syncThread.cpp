#include "syncThread.hpp"
#include <cstddef>
#include <pthread.h>
#include "../Common/package.hpp"
#include "../Common/package_functions.hpp"
#include "DadosConexao.hpp"
#include <string>
#include "../Common/defines.hpp"
#include "comunicacaoCliente.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_file.hpp"
#include "auxiliaresCliente.hpp"

extern DadosConexao dados_conexao;
extern PackageChangeEvent previousSyncedChangeEvent;
extern pthread_mutex_t previousSyncedChangeEventLock;

void exitSyncThread(void)
{
    // Limpa campo sync_thread de dados_conexao indicando que a thread não está mais executando
    dados_conexao.sync_thread = std::nullopt;

    // Socket de eventos não será fechado pois eventThread também o utiliza

    pthread_exit(NULL);
}

void *syncThread(void *)
{
    std::vector<char> fileContentBuffer;
    Package package;
    std::string path_base = std::string(PREFIXO_DIRETORIO);
    std::string path;
    std::string old_path;
    std::string new_path;
    path_base.append(dados_conexao.nome_usuario);
    path_base.append("/");
    FILE *fp;

    while (true)
    {
        if (read_package_from_socket(dados_conexao.event_connection_socket, package, fileContentBuffer))
        {
            printf("[sync] Erro ao ler pacote do servidor.\n");
            break;
        }

        // Ignora pacotes que não sejam de eventos
        if (package.package_type != CHANGE_EVENT)
        {
            printf("[sync] Pacote inesperado: 0x%02x.\n", (uint8_t)package.package_type);
            continue;
        }

        auto changeEvent = package.package_specific.changeEvent;

        // previousSyncedChangeEvent é acessado por eventThread
        pthread_mutex_lock(&previousSyncedChangeEventLock);
        previousSyncedChangeEvent = changeEvent;
        pthread_mutex_unlock(&previousSyncedChangeEventLock);

        switch (changeEvent.event)
        {
        case FILE_DELETED:
            path = path_base;
            path.append(changeEvent.filename1);

            if (remove(path.c_str()))
            {
                printf("[sync] Erro ao remover arquivo \"%s\".\n", changeEvent.filename1);
            }
            break;
        case FILE_CREATED:
            path = path_base;
            path.append(changeEvent.filename1);
            fp = fopen(path.c_str(), "w");

            if (!fp)
            {
                printf("[sync] Erro ao criar arquivo \"%s\".\n", changeEvent.filename1);
            }
            else
            {
                fclose(fp);
            }
            break;
        case FILE_MODIFIED:
            path = path_base;
            path.append(changeEvent.filename1);

            // Utilizamos socket de eventos para receber o arquivo modificado
            pthread_mutex_lock(dados_conexao.event_connection_socket_lock);
            if (download_file(dados_conexao.main_connection_socket, changeEvent.filename1, path.c_str()))
            {
                printf("[sync] Nao foi possivel baixar versao modificada do arquivo \"%s\".\n", changeEvent.filename1);
            }
            pthread_mutex_unlock(dados_conexao.event_connection_socket_lock);

            break;
        case FILE_RENAME:
            old_path = path_base;
            new_path = path_base;

            old_path.append(changeEvent.filename1);
            new_path.append(changeEvent.filename2);

            if (rename(old_path.c_str(), new_path.c_str()))
            {
                printf("[sync] Erro ao renomear arquivo de \"%s\" para \"%s\".\n", changeEvent.filename1, changeEvent.filename2);
            }

            break;
        default:
            printf("[sync] Evento desconhecido: 0x%02x\n", (uint8_t)changeEvent.event);
            continue;
        }
    }

    exitSyncThread();

    return NULL;
}
