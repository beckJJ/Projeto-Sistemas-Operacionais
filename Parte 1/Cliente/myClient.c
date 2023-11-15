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

#include "comunicacaoCliente.h"
#include "interfaceCliente.h"
#include "auxiliaresCliente.h"

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

    dados_conexao.socket_id = conecta_servidor(dados_conexao); /* Inicia a conexao com o servidor, via socket. */
    analisa_diretorio(dados_conexao.nome_usuario);             /* Cria ou carrega o diretorio sync_dir_<usuario> */

    while (1)
    {
        char *ret;

        limpaTela();
        menu_principal(dados_conexao); /* Exibe o menu principal ao usuario, para digitar os comandos. */

        ret = fgets(dados_conexao.comando, sizeof(dados_conexao.comando), stdin); /* Obtem o comando inserido pelo usuario. */

        // Erro ou EOF
        if (ret == NULL) {
            break;
        }

        dados_conexao.comando[strnlen(dados_conexao.comando, sizeof(dados_conexao.comando)) - 1] = '\0';

        executa_comando(dados_conexao); /* Identifica e executa o comando inserido pelo usuario. */
    }

    limpaTela();
    return 0;
}
