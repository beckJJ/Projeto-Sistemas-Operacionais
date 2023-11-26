#include <stdio.h>
#include <string.h>
#include <iostream>
#include "../Common/package.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/functions.hpp"
#include "../Common/defines.hpp"
#include "../Common/package_commands.hpp"
#include "comunicacaoCliente.hpp"
#include "interfaceCliente.hpp"
#include "inotifyThread.hpp"

int main(int argc, char *argv[])
{
    if (argc != QUANTIDADE_PARAMETROS_MYCLIENT + 1) /* Controle do numero de correto de parametros. */
    {
        printf("Comando invalido! Numero de parametros incorreto!\n");
        printf("Forma correta:\n./myClient <username> <server_ip_address> <port>\n");
        exit(EXIT_FAILURE);
    }

    /* Copia os dados da conexao, passados como parametro pelo usuario. */
    DadosConexao dados_conexao;
    strcpy(dados_conexao.nome_usuario, argv[1]);
    strcpy(dados_conexao.endereco_ip, argv[2]);
    strcpy(dados_conexao.numero_porta, argv[3]);

    std::string sync_dir_path = PREFIXO_DIRETORIO;
    sync_dir_path.append(dados_conexao.nome_usuario);

    if (create_dir_if_not_exists(sync_dir_path.c_str()))
    {
        printf("Erro criar diretorio sync dir do usuario.\n");
        exit(EXIT_FAILURE);
    }

    dados_conexao.socket_id = conecta_servidor(&dados_conexao); /* Inicia a conexao com o servidor, via socket. */

    auto package = Package(PackageUserIndentification(dados_conexao.nome_usuario));
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(dados_conexao.socket_id, package, fileContentBuffer))
    {
        printf("Nao foi possivel enviar nome de usuario.\n");
        exit(EXIT_FAILURE);
    }

    if (read_package_from_socket(dados_conexao.socket_id, package, fileContentBuffer))
    {
        printf("Erro ao ler resposta inicial do servidor.\n");
        exit(EXIT_FAILURE);
    }

    if (package.package_type != USER_INDENTIFICATION_RESPONSE)
    {
        printf("Resposta invalida do servidor.\n");
        exit(EXIT_FAILURE);
    }

    if (package.package_specific.userIdentificationResponse.status == REJECTED)
    {
        printf("Nao foi possivel se registrar como dispositivo para usuario \"%s\".\n", dados_conexao.nome_usuario);
        exit(EXIT_FAILURE);
    }

    uint8_t device_id = package.package_specific.userIdentificationResponse.deviceID;

    // Usuário já está conectado como dispositivo, iniciamos a thread que monitora eventos do inotify
    pthread_t thread;
    ThreadArg thread_arg{dados_conexao.socket_id, device_id, dados_conexao.nome_usuario};

    pthread_create(&thread, NULL, inotifyThread, &thread_arg);

    while (1)
    {
        char *ret;

        limpaTela();
        menu_principal(&dados_conexao); /* Exibe o menu principal ao usuario, para digitar os comandos. */

        ret = fgets(dados_conexao.comando, sizeof(dados_conexao.comando), stdin); /* Obtem o comando inserido pelo usuario. */

        // Erro ou EOF
        if (ret == NULL)
        {
            break;
        }

        dados_conexao.comando[strnlen(dados_conexao.comando, sizeof(dados_conexao.comando)) - 1] = '\0';

        executa_comando(&dados_conexao); /* Identifica e executa o comando inserido pelo usuario. */
    }

    limpaTela();

    pthread_cancel(thread);

    return 0;
}
