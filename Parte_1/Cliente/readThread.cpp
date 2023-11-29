#include "readThread.hpp"
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

void exitReadThread(void)
{
    // Limpa campo sync_thread de dados_conexao indicando que a thread não está mais executando
    dados_conexao.sync_thread = std::nullopt;

    pthread_exit(NULL);
}

// Lê uma sequência de pacotes de PackageFileList, package já é um pacote que deve ser processado
void handleFileList(Package &package)
{
    size_t total_files = package.package_specific.fileList.count;
    std::vector<char> fileContentBuffer;

    dados_conexao.file_list.clear();

    if (total_files == 0)
    {
        return;
    }

    dados_conexao.file_list.push_back(package.package_specific.fileList.file);

    while (total_files != dados_conexao.file_list.size())
    {
        if (read_package_from_socket(dados_conexao.socket, package, fileContentBuffer))
        {
            dados_conexao.file_list.clear();
            return;
        }

        // Pacote inesperado
        if (package.package_type != FILE_LIST)
        {
            dados_conexao.file_list.clear();
            return;
        }

        // Adiciona file
        dados_conexao.file_list.push_back(package.package_specific.fileList.file);
    }
}

// Lida com pacote PackageChangeEvent
void handleChangeEvent(PackageChangeEvent &changeEvent, std::string &path_base)
{
    std::vector<char> fileContentBuffer;
    std::string old_path = path_base;
    std::string new_path = path_base;
    std::string path;
    FILE *fp;

    // previousSyncedChangeEvent é acessado por eventThread
    pthread_mutex_lock(&previousSyncedChangeEventLock);
    previousSyncedChangeEvent = changeEvent;
    pthread_mutex_unlock(&previousSyncedChangeEventLock);

    switch (changeEvent.event)
    {
    // Remove arquivo de sync dir local
    case FILE_DELETED:
        path = path_base;
        path.append(changeEvent.filename1);

        if (remove(path.c_str()))
        {
            printf("Erro ao remover arquivo \"%s\".\n", changeEvent.filename1);
        }
        break;
    // Cria arquivo no sync dir local
    case FILE_CREATED:
        path = path_base;
        path.append(changeEvent.filename1);
        fp = fopen(path.c_str(), "w");

        if (!fp)
        {
            printf("Erro ao criar arquivo \"%s\".\n", changeEvent.filename1);
        }
        else
        {
            fclose(fp);
        }
        break;
    // Baixa versão atualizada do arquivo no sync dir local
    case FILE_MODIFIED:
        path = path_base;
        path.append(changeEvent.filename1);

        // Evita que usuário escreva pacotes enquanto o conteúdo do arquivo é recebido
        pthread_mutex_lock(dados_conexao.socket_lock);

        // Após o evento FILE_MODIFIED haverá pacotes de upload de arquivo
        if (read_upload_file_and_save(dados_conexao.socket, path.c_str()))
        {
            printf("Nao foi possivel baixar versao modificada do arquivo \"%s\".\n", changeEvent.filename1);
        }

        pthread_mutex_unlock(dados_conexao.socket_lock);

        break;
    // Renomeia arquivo no sync dir local
    case FILE_RENAME:
        old_path = path_base;
        new_path = path_base;

        old_path.append(changeEvent.filename1);
        new_path.append(changeEvent.filename2);

        if (rename(old_path.c_str(), new_path.c_str()))
        {
            printf("Erro ao renomear arquivo de \"%s\" para \"%s\".\n", changeEvent.filename1, changeEvent.filename2);
        }

        break;
    default:
        printf("Evento desconhecido: 0x%02x\n", (uint8_t)changeEvent.event);
        break;
    }
}

void *readThread(void *)
{
    std::vector<char> fileContentBuffer;
    Package package;
    std::string path_base = std::string(PREFIXO_DIRETORIO);
    path_base.append(dados_conexao.nome_usuario);
    path_base.append("/");

    while (true)
    {
        if (read_package_from_socket(dados_conexao.socket, package, fileContentBuffer))
        {
            printf("Erro ao ler pacote do servidor.\n");
            break;
        }

        switch (package.package_type)
        {
        // Resposta de PackageRequestFileList, devemos salvar resposta em file_list e sinalizar condição
        case FILE_LIST:
            pthread_mutex_lock(dados_conexao.file_list_lock);

            handleFileList(package);
            dados_conexao.is_file_list_readed = true;

            pthread_mutex_unlock(dados_conexao.file_list_lock);
            pthread_cond_signal(dados_conexao.file_list_cond);
            break;
        // Resposta de PackageRequestFile, informa que não foi possível baixar
        case FILE_NOT_FOUND:
            printf("Arquivo requisitado nao existe.\n");
            break;
        // Resposta de PackageRequestFile, salva o conteúdo no cwd
        case UPLOAD_FILE:
            if (read_file_and_save(dados_conexao.socket, package.package_specific.uploadFile.name))
            {
                printf("Erro ao receber arquivo \"%s\".\n", package.package_specific.uploadFile.name);
            }
            break;
        // Houve alguma alteração nos arquivos do servidor
        case CHANGE_EVENT:
            handleChangeEvent(package.package_specific.changeEvent, path_base);
            break;
        default:
            printf("[handle_other] Pacote desconhecido: 0x%02x\n", (uint8_t)package.package_type);
            break;
        }
    }

    exitReadThread();

    return NULL;
}
