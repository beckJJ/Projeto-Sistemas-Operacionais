#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <cstring>
#include <strings.h>
#include <libgen.h>
#include <iostream>
#include "../Common/package.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_file.hpp"
#include "DadosConexao.hpp"
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

    char buffer[MAX_DATA_SIZE];

    bool success = true;

    FILE *fin = fopen(path_in.c_str(), "rb");

    if (!fin)
    {
        printf("Nao foi possivel abrir arquivo \"%s\" para leitura.\n", path_in.c_str());
    }

    FILE *fout = fopen(path_out.c_str(), "wb");

    if (!fout)
    {
        printf("Nao foi possivel abrir arquivo \"%s\" para escrita.\n", path_out.c_str());
    }

    // Lê todo arquivo fin e escreve o conteúdo em fout
    while (!feof(fin))
    {
        auto total_read = fread(buffer, sizeof(char), MAX_DATA_SIZE, fin);

        if (ferror(fin))
        {
            success = false;
            break;
        }

        fwrite(buffer, sizeof(char), total_read, fout);

        if (ferror(fout))
        {
            success = false;
            break;
        }
    }

    fclose(fin);
    fclose(fout);

    if (success)
    {
        printf("Arquivo \"%s\" copiado para sync dir local com sucesso.\n", path_in.c_str());
    }
    else
    {
        printf("Algum erro ocorreu durante a copia do arquivo \"%s\".\n", path_in.c_str());
    }
}

// Pede arquivo do servidor, caso esteja disponível, será salvo no cwd atual
void download(DadosConexao &dados_conexao)
{
    auto strings = splitComando(dados_conexao.comando);
    std::string filename = strings[1];

    switch (download_file(dados_conexao.main_connection_socket, filename.c_str(), filename.c_str()))
    {
    case SUCCESS:
        printf("Arquivo \"%s\" baixado do servidor com sucesso.\n", filename.c_str());
        break;
    case DOWNLOAD_FILE_FILE_NOT_FOUND:
        printf("Arquivo \"%s\" nao existe no servidor.\n", filename.c_str());
        break;
    case DOWNLOAD_FILE_ERROR:
    default:
        printf("Erro ao baixar arquivo \"%s\" do servidor (pode nao existir).\n", filename.c_str());
        break;
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

// Pede lista de arquivos para o servidor e exibe a lista
void list_server(DadosConexao &dados_conexao)
{
    Package package = Package(PackageRequestFileList());
    std::vector<char> fileContentBuffer;

    // Pede listagem de arquivos do servidor
    if (write_package_to_socket(dados_conexao.main_connection_socket, package, fileContentBuffer))
    {
        printf("Erro ao enviar pacote para pedir listagem do servidor.\n");
        return;
    }

    auto files = read_file_list(dados_conexao.main_connection_socket);

    if (!files.has_value())
    {
        printf("Nao foi possivel receber listagem de arquivos.\n");
        return;
    }

    // Exibe listagem recebida
    print_files(files.value());
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

    print_files(files.value());
}
