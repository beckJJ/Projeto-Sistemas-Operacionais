#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <strings.h>
#include <libgen.h>
#include <iostream>
#include "../Common/package.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_commands.hpp"
#include "DadosConexao.h"
#include "interfaceCliente.hpp"
#include "auxiliaresCliente.hpp"

int conecta_servidor(DadosConexao *dados_conexao) /* Inicia a conexao com o servidor, via socket. */
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(dados_conexao->endereco_ip); /* Obtem informacao do host, a partir do seu endereco IP. */

    if (server == NULL)
    {
        printf("Erro! Nenhum servidor com o IP %s foi encontrado!\n", dados_conexao->endereco_ip);
        exit(EXIT_FAILURE);
    }

    int socket_id;

    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) /* Inicia a utilizacao de um socket, guardando o valor inteiro que o referencia. */
    {
        printf("Erro! Nao foi possivel iniciar utilizacao do socket do cliente!\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;                                /* Identifica o protocolo de rede a ser utilizado. */
    serv_addr.sin_port = htons(atoi(dados_conexao->numero_porta)); /* Identifica o numero de porta a ser utilizado. */
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(socket_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) /* Solicita abertura de conexao com o servidor. */
    {
        printf("Erro! Nao foi possivel estabelecer conexao com o socket do servidor!\n");
        exit(EXIT_FAILURE);
    }

    return socket_id; /* Retorna o identificador do socket utilizado para estabelecer conexao com servidor. */
}

void upload(DadosConexao *dados_conexao) /* Realiza o envio de um arquivo para o diretorio do usuario, presente no servidor. */
{
    Package package;
    struct stat info;
    std::vector<char> fileContentBuffer;

    auto strings = splitComando(dados_conexao->comando);

    if (stat(strings[1], &info) != 0)
    {
        printf("Erro ao fazer stat do arquivo \"%s\".\n", strings[1]);
        return;
    }

    std::string path = strings[1];

    package = Package(PackageUploadFile(File(
        info.st_size,
        info.st_mtime,
        info.st_atime,
        info.st_ctime,
        basename(strings[1]))));

    if (write_package_to_socket(dados_conexao->socket_id, package, fileContentBuffer))
    {
        printf("Nao foi possivel enviar PackageUploadFile header.\n");
        return;
    }

    if (send_file(dados_conexao->socket_id, path.c_str()))
    {
        printf("Algum erro ocorreu ao enviar o arquivo.\n");
        return;
    }

    printf("Arquivo \"%s\" enviado para o servidor com sucesso.\n", path.c_str());
}

/* Recebe um arquivo do servidor, e copia para o diretorio de onde a aplicacao cliente foi chamada. */
void download(DadosConexao *dados_conexao)
{
    auto strings = splitComando(dados_conexao->comando);
    std::string filename = strings[1];
    Package package = Package(PackageRequestFile(filename.c_str()));
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(dados_conexao->socket_id, package, fileContentBuffer))
    {
        printf("Erro ao enviar pacote para pedir arquivo.\n");
        return;
    }

    if (read_package_from_socket(dados_conexao->socket_id, package, fileContentBuffer))
    {
        printf("Erro ao ler resposta de requisição de arquivo.\n");
        return;
    }

    if (package.package_type == FILE_NOT_FOUND)
    {
        printf("Arquivo requisitado nao existe no servidor.\n");
        return;
    }

    if (read_file_and_save(dados_conexao->socket_id, filename.c_str()))
    {
        printf("Erro ao ler e salvar o arquivo do servidor.\n");
        return;
    }

    printf("Arquivo \"%s\" baixado do servidor com sucesso.\n", filename.c_str());
}

void delete_cmd(DadosConexao *dados_conexao) /* Delete um arquivo presente na maquina local do usuario, no diretorio sync_dir_<usuario> */
{
    auto strings = splitComando(dados_conexao->comando);
    std::string filename = strings[1];
    std::string path = PREFIXO_DIRETORIO;
    path.append(dados_conexao->nome_usuario);
    path.append("/");
    path.append(filename);

    if (remove(path.c_str()))
    {
        printf("Nao foi possivel remover arquivo \"%s\"\n", filename.c_str());
        return;
    }

    printf("Arquivo \"%s\" deletado do diretorio local com sucesso.\n", filename.c_str());
}

/* Lista os arquivos armazenados no repositorio remoto do servidor, associados ao usuario. */
void list_server(DadosConexao *dados_conexao)
{
    Package package = Package(PackageRequestFileList());
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(dados_conexao->socket_id, package, fileContentBuffer))
    {
        printf("Erro ao enviar pacote para pedir listagem do servidor.\n");
        return;
    }

    auto files = read_file_list(dados_conexao->socket_id);

    if (!files.has_value())
    {
        printf("Nao foi possivel receber listagem de arquivos.\n");
        return;
    }

    print_files(files.value());
}

void list_client(DadosConexao *dados_conexao) /* Lista todos os arquivos do repositorio local syn_dir_<usuario>, junto com os MAC times. */
{
    std::string path = PREFIXO_DIRETORIO;
    path.append(dados_conexao->nome_usuario);

    auto files = list_dir(path.c_str());

    if (!files.has_value())
    {
        printf("Nao foi possivel listar arquivos em \"%s\".\n", path.c_str());
        return;
    }

    print_files(files.value());
}
