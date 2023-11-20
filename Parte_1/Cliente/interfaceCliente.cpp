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

#include "comunicacaoCliente.hpp"
#include "interfaceCliente.hpp"
#include "auxiliaresCliente.hpp"

void limpaTela() /* Limpa o conteudo do terminal que executa a aplicacao. */
{
    system("clear");
}

void menu_help() /* Exibe uma lista de comandos possiveis de serem executados, informando a sua sintaxe e a sua utilidade. */
{
    limpaTela();
    printf("\nLISTA DE COMANDOS:\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 1:");
    printf("\n\nSINTAXE:\nupload <path/filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Envia o arquivo filename.ext para o servidor, colocando-o no 'sync_dir' do\n");
    printf("servidor e propagando-o para todos os dispositivos do usuario.\n");
    printf("e.g. upload /home/user/MyFolder/filename.ext\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 2:");
    printf("\n\nSINTAXE:\ndownload <filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Faz uma copia nao sincronizada do arquivo filename.ext do servidor para\n");
    printf("o diretorio local (de onde o servidor foi chamado).\n");
    printf("e.g. download mySpreadsheet.xlsx\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 3:");
    printf("\n\nSINTAXE:\ndelete <filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Exclui o arquivo <filename.ext> de 'sync_dir'.\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 4:");
    printf("\n\nSINTAXE:\nlist_server");
    printf("\n\nDESCRICAO:\n");
    printf("Lista os arquivos salvos no servidor, associados ao usuario.\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 5:");
    printf("\n\nSINTAXE:\nlist_client");
    printf("\n\nDESCRICAO:\n");
    printf("Lista os arquivos salvos no diretorio 'sync_dir'\n, do dispositivo local do usuario.\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 6:");
    printf("\n\nSINTAXE:\nexit");
    printf("\n\nDESCRICAO:\n");
    printf("Fecha a sessao com o servidor.\n");

    printf("\n------------------------------------------------------------------------------\n");
}

/* Mostra as informacoes da conexao estabelecida com o servidor, e solicita um comando ao usuario. */
void menu_principal(DadosConexao dados_conexao) 
{
    printf("USUARIO CONECTADO:\n%s\n", dados_conexao.nome_usuario);
    printf("\nIP DO SERVIDOR:\n%s\n", dados_conexao.endereco_ip);
    printf("\nPORTA:\n%s\n", dados_conexao.numero_porta);
    printf("\nDigite 'help' para ver uma lista de comandos.\n");
    printf("Digite 'exit' para sair.\n");
    
    printf("\nDIGITE O COMANDO:\n");
}

/* Identifica qual o comando inserido pelo usuario, verifica se o numero de parametros esta correto e inicia a execucao. */
void executa_comando(DadosConexao dados_conexao)
{
    //printf("COMANDO INSERIDO:\n%s\n", dados_conexao.comando);
    
    if (strncmp(dados_conexao.comando,COMANDO_UPLOAD,strlen(COMANDO_UPLOAD)) == 0)
    {
    	if (verificaParametros(dados_conexao.comando,QUANTIDADE_PARAMETROS_UPLOAD) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		upload(dados_conexao); /* Realiza o envio de um arquivo para o diretorio do usuario, presente no servidor. */
    }

    else if (strncmp(dados_conexao.comando,COMANDO_DOWNLOAD,strlen(COMANDO_DOWNLOAD)) == 0)
    {
    	if (verificaParametros(dados_conexao.comando,QUANTIDADE_PARAMETROS_DOWNLOAD) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		download(dados_conexao); /* Recebe um arquivo do servidor, e copia para o diretorio de onde a aplicacao cliente foi chamada. */
    }

    else if (strncmp(dados_conexao.comando,COMANDO_DELETE,strlen(COMANDO_DELETE)) == 0)
    {
    	if (verificaParametros(dados_conexao.comando,QUANTIDADE_PARAMETROS_DELETE) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		delete_cmd(dados_conexao); /* Deleta um arquivo presente na maquina local do usuario, no diretorio sync_dir_<usuario> */
    }

    else if (strcmp(dados_conexao.comando,COMANDO_LISTSERVER) == 0)
        list_server(dados_conexao); /* Lista os arquivos armazenados no repositorio remoto do servidor, associados ao usuario. */

    else if (strcmp(dados_conexao.comando,COMANDO_LISTCLIENT) == 0)
    	list_client(dados_conexao); /* Lista todos os arquivos do repositorio local syn_dir_<usuario>, junto com os MAC times. */

    else if (strcmp(dados_conexao.comando,COMANDO_EXIT) == 0) /* Encerra a conexao com o servidor. */
    {
        limpaTela();
        close(dados_conexao.socket_id);
        exit(0);
    }
    	
    else if (strcmp(dados_conexao.comando,COMANDO_HELP) == 0)
        menu_help(); /* Exibe uma lista de comandos possiveis de serem executados, informando a sua sintaxe e a sua utilidade. */

    else
        printf("\nComando invalido!\n");
    
    printf("\nPressione qualquer tecla para continuar.\n");
    getchar();
}
