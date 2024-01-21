#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <cstring>
#include <strings.h>
#include <optional>
#include <algorithm>
#include <libgen.h>
#include <iostream>
#include "../Common/package.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_file.hpp"
#include "../Common/DadosConexao.hpp"
#include "interfaceCliente.hpp"
#include "auxiliaresCliente.hpp"

// Conecta-se com o servidor
std::optional<int> conecta_servidor(DadosConexao &dados_conexao)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(dados_conexao.endereco_ip); /* Obtem informacao do host, a partir do seu endereco IP. */

    if (server == NULL)
    {
        printf("Erro! Nenhum servidor com o IP %s foi encontrado!\n", dados_conexao.endereco_ip);
        return std::nullopt;
    }

    int socket_id;

    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) /* Inicia a utilizacao de um socket, guardando o valor inteiro que o referencia. */
    {
        printf("Erro! Nao foi possivel iniciar utilizacao do socket do cliente!\n");
        return std::nullopt;
    }

    serv_addr.sin_family = AF_INET;                               /* Identifica o protocolo de rede a ser utilizado. */
    serv_addr.sin_port = htons(atoi(dados_conexao.numero_porta)); /* Identifica o numero de porta a ser utilizado. */
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(socket_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) /* Solicita abertura de conexao com o servidor. */
    {
        printf("Erro! Nao foi possivel estabelecer conexao com o socket do servidor!\n");
        return std::nullopt;
    }

    return socket_id;
}

int copy_file(const char *path_orig, const char *path_dest)
{
    int ret = 0;
    char buffer[MAX_DATA_SIZE];
    FILE *fin = fopen(path_orig, "rb");

    if (!fin)
    {
        return 1;
    }

    FILE *fout = fopen(path_dest, "wb");

    if (!fout)
    {
        fclose(fin);
        return 1;
    }

    // Lê todo arquivo fin e escreve o conteúdo em fout
    while (!feof(fin))
    {
        auto total_read = fread(buffer, sizeof(char), MAX_DATA_SIZE, fin);

        if (ferror(fin))
        {
            ret = 1;
            break;
        }

        fwrite(buffer, sizeof(char), total_read, fout);

        if (ferror(fout))
        {
            ret = 1;
            break;
        }
    }

    fclose(fin);
    fclose(fout);

    return ret;
}

// Copia o arquivo informado pelo usuário para o diretório sync_dir local, thread de eventos que
//   observa eventos do inotify deverá gerar os eventos apropriados
void upload(DadosConexao &dados_conexao)
{
    auto strings = splitComando(dados_conexao.comando);
    std::string path_in = strings[1];
    std::string path_out = PREFIXO_DIRETORIO;
    path_out.append(dados_conexao.nome_usuario);
    path_out.append("/");

    // Obtém basename do path, usuário pode ter passado caminho com '/'
    char *filename = basename(strings[1]);
    path_out.append(filename);

    if (copy_file(path_in.c_str(), path_out.c_str()))
    {
        printf("Algum erro ocorreu durante a copia do arquivo \"%s\".\n", path_in.c_str());
    }
    else
    {
        printf("Arquivo \"%s\" copiado para sync dir local com sucesso, sera enviado apos evento.\n", path_in.c_str());
    }
}

// Copia arquivo do sync_dir local para cwd
void download(DadosConexao &dados_conexao)
{
    auto strings = splitComando(dados_conexao.comando);
    std::string filename = strings[1];
    std::string path_sync_dir = PREFIXO_DIRETORIO;
    path_sync_dir.append(dados_conexao.nome_usuario);
    path_sync_dir.append("/");
    path_sync_dir.append(filename);

    // O sync_dir local deve estar sincronizado
    if (copy_file(path_sync_dir.c_str(), filename.c_str()))
    {
        printf("Nao foi possivel copiar o arquivo \"%s\" do sync dir local para pasta atual.\n", filename.c_str());
    }
    else
    {
        printf("Arquivo \"%s\" copiado para diretorio atual com sucesso.\n", filename.c_str());
    }
}

// Deleta um arquivo em sync_dir
void delete_cmd(DadosConexao &dados_conexao)
{
    auto strings = splitComando(dados_conexao.comando);
    std::string filename = strings[1];
    std::string path = PREFIXO_DIRETORIO;
    path.append(dados_conexao.nome_usuario);
    path.append("/");
    path.append(filename);

    if (remove(path.c_str()))
    {
        printf("Nao foi possivel remover arquivo \"%s\"\n", filename.c_str());
        return;
    }

    printf("Arquivo \"%s\" deletado do diretorio local com sucesso.\n", filename.c_str());
}

// Pede lista de arquivos para o servidor e exibe a lista após a condição
//   dados_conexao.is_file_list_read ter sido completa pela thread de leitura
void list_server(DadosConexao &dados_conexao)
{
    Package package = Package(PackageRequestFileList());
    std::vector<char> fileContentBuffer;

    // Socket será usado
    pthread_mutex_lock(dados_conexao.socket_lock);

    // Pede listagem de arquivos do servidor
    int ret = write_package_to_socket(dados_conexao.socket, package, fileContentBuffer);

    pthread_mutex_unlock(dados_conexao.socket_lock);

    if (ret)
    {
        printf("Erro ao enviar pacote para pedir listagem do servidor.\n");
        return;
    }

    // Leremos a lista de arquivos
    pthread_mutex_lock(dados_conexao.file_list_lock);

    dados_conexao.is_file_list_read = false;

    // Espera que a thread de leitura leia os pacotes PackageFileList que devem ser enviados em
    //   resposta a requisição feita acima
    while (!dados_conexao.is_file_list_read)
    {
        pthread_cond_wait(dados_conexao.file_list_cond, dados_conexao.file_list_lock);
    }

    // Ordena para melhor exibição
    std::sort(dados_conexao.file_list.begin(), dados_conexao.file_list.end());
    print_files(dados_conexao.file_list);

    pthread_mutex_unlock(dados_conexao.file_list_lock);
}

// Lista arquivos presentes no sync_dir local
void list_client(DadosConexao &dados_conexao)
{
    std::string path = PREFIXO_DIRETORIO;
    path.append(dados_conexao.nome_usuario);

    // Lista diretório local
    auto files = list_dir(path.c_str());

    if (!files.has_value())
    {
        printf("Nao foi possivel listar arquivos em \"%s\".\n", path.c_str());
        return;
    }

    // Ordena para melhor exibição
    std::sort(files.value().begin(), files.value().end());
    print_files(files.value());
}
