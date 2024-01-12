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

int limpaTela() /* Limpa o conteudo do terminal que executa a aplicacao. */
{
#if LIMPA_TELA_NOOP
    return 0;
#else
    return system("clear");
#endif
}

void menu_help_comandos()
{
    limpaTela();
    printf("\nPara uma explicacao sobre cada comando, digite \"help <comando>\"\n\n");
    printf("COMANDOS DISPONIVEIS:\n\n");
    printf("1.  upload <path/filename.ext>\n");
    printf("2.  download <filename.ext>\n");
    printf("3.  delete <filename.ext>\n");
    printf("4.  list_server\n");
    printf("5.  list_client\n");
    printf("6.  exit\n");
    printf("7.  help\n");
}

void menu_help(int comando_id) /* Exibe uma lista de comandos possiveis de serem executados, informando a sua sintaxe e a sua utilidade. */
{
    limpaTela();
        printf("\n------------------------------------------------------------------------------\n");
    switch(comando_id) {
    case 1:
        printf("\nCOMANDO 1:");
        printf("\n\nSINTAXE:\nupload <path/filename.ext>");
        printf("\n\nDESCRICAO:\n");
        printf("Envia o arquivo filename.ext para o servidor, colocando-o no 'sync_dir' do\n");
        printf("servidor e propagando-o para todos os dispositivos do usuario.\n");
        printf("e.g. upload /home/user/MyFolder/filename.ext\n");
        break;
    case 2:
        printf("\nCOMANDO 2:");
        printf("\n\nSINTAXE:\ndownload <filename.ext>");
        printf("\n\nDESCRICAO:\n");
        printf("Faz uma copia nao sincronizada do arquivo filename.ext do servidor para\n");
        printf("o diretorio local (de onde o servidor foi chamado).\n");
        printf("e.g. download mySpreadsheet.xlsx\n");
        break;
    case 3:
        printf("\nCOMANDO 3:");
        printf("\n\nSINTAXE:\ndelete <filename.ext>");
        printf("\n\nDESCRICAO:\n");
        printf("Exclui o arquivo <filename.ext> de 'sync_dir'.\n");
        break;
    case 4:
        printf("\nCOMANDO 4:");
        printf("\n\nSINTAXE:\nlist_server");
        printf("\n\nDESCRICAO:\n");
        printf("Lista os arquivos salvos no servidor, associados ao usuario.\n");
        break;
    case 5:    
        printf("\nCOMANDO 5:");
        printf("\n\nSINTAXE:\nlist_client");
        printf("\n\nDESCRICAO:\n");
        printf("Lista os arquivos salvos no diretorio 'sync_dir' do dispositivo local do usuario.\n");
        break;
    case 6:
        printf("\nCOMANDO 6:");
        printf("\n\nSINTAXE:\nexit");
        printf("\n\nDESCRICAO:\n");
        printf("Fecha a sessao com o servidor.\n");
        break;
    case 7:
        printf("\nCOMANDO 7:");
        printf("\n\nSINTAXE:\nhelp <comando>");
        printf("\n\nDESCRICAO:\n");
        printf("Imprime na tela uma explicacao de como utilizar o comando informado.\n");
        printf("Pode ser informado tanto o nome do comando quanto o id do comando.\n");
        printf("e.g. help upload\n");
        break;
    default:
        printf("\nCOMANDO NAO ENCONTRADO!");
        printf("\n\nCOMANDOS DISPONIVEIS:\n\n");
        printf("1.  upload <path/filename.ext>\n");
        printf("2.  download <filename.ext>\n");
        printf("3.  delete <filename.ext>\n");
        printf("4.  list_server\n");
        printf("5.  list_client\n");
        printf("6.  exit\n");
        printf("7.  help <comando>\n");
        break;
    }
    printf("\n------------------------------------------------------------------------------\n");
}

/* Mostra as informacoes da conexao estabelecida com o servidor, e solicita um comando ao usuario. */
void menu_principal(DadosConexao &dados_conexao)
{
    printf("USUARIO CONECTADO:\n%s\n", dados_conexao.nome_usuario);
    printf("\nIP DO SERVIDOR:\n%s\n", dados_conexao.endereco_ip);
    printf("\nPORTA:\n%s\n", dados_conexao.numero_porta);
    printf("\nDigite 'help' para ver uma lista de comandos.\n");
    printf("Digite 'exit' para sair.\n");

    printf("\nDIGITE O COMANDO:\n");
}

/* Identifica qual o comando inserido pelo usuario, verifica se o numero de parametros esta correto e inicia a execucao. */
int executa_comando(DadosConexao &dados_conexao)
{
    printf("COMANDO INSERIDO:\n%s\n", dados_conexao.comando);
    limpaTela();

    if (strncmp(dados_conexao.comando, COMANDO_UPLOAD, strlen(COMANDO_UPLOAD)) == 0)
    {
        if (verificaParametros(dados_conexao.comando, QUANTIDADE_PARAMETROS_UPLOAD) == 0)
            printf("\nComando invalido! Numero de parametros incorreto!\n");
        else
            upload(dados_conexao); /* Realiza o envio de um arquivo para o diretorio do usuario, presente no servidor. */
    }

    else if (strncmp(dados_conexao.comando, COMANDO_DOWNLOAD, strlen(COMANDO_DOWNLOAD)) == 0)
    {
        if (verificaParametros(dados_conexao.comando, QUANTIDADE_PARAMETROS_DOWNLOAD) == 0)
            printf("\nComando invalido! Numero de parametros incorreto!\n");
        else
            download(dados_conexao); /* Recebe um arquivo do servidor, e copia para o diretorio de onde a aplicacao cliente foi chamada. */
    }

    else if (strncmp(dados_conexao.comando, COMANDO_DELETE, strlen(COMANDO_DELETE)) == 0)
    {
        if (verificaParametros(dados_conexao.comando, QUANTIDADE_PARAMETROS_DELETE) == 0)
            printf("\nComando invalido! Numero de parametros incorreto!\n");
        else
            delete_cmd(dados_conexao); /* Deleta um arquivo presente na maquina local do usuario, no diretorio sync_dir_<usuario> */
    }

    else if (strcmp(dados_conexao.comando, COMANDO_LISTSERVER) == 0)
        list_server(dados_conexao); /* Lista os arquivos armazenados no repositorio remoto do servidor, associados ao usuario. */

    else if (strcmp(dados_conexao.comando, COMANDO_LISTCLIENT) == 0)
        list_client(dados_conexao); /* Lista todos os arquivos do repositorio local syn_dir_<usuario>, junto com os MAC times. */

    else if (strcmp(dados_conexao.comando, COMANDO_EXIT) == 0) /* Encerra a conexao com o servidor. */
    {
        limpaTela();
        return 1;
    }

    else if (strncmp(dados_conexao.comando, COMANDO_HELP, 4) == 0) {
        std::vector<char*> strings = splitComando(dados_conexao.comando);
        if (strings.size() > 1) {
            char *help_comando = strings[1];
            int id_comando;
            if (strcmp(help_comando, COMANDO_UPLOAD) == 0) {
                id_comando = ID_COMANDO_UPLOAD;
            } else if (strcmp(help_comando, COMANDO_DOWNLOAD) == 0) {
                id_comando = ID_COMANDO_DOWNLOAD;
            } else if (strcmp(help_comando, COMANDO_DELETE) == 0) {
                id_comando = ID_COMANDO_DELETE;
            } else if (strcmp(help_comando, COMANDO_LISTSERVER) == 0) {
                id_comando = ID_COMANDO_LISTSERVER;
            } else if (strcmp(help_comando, COMANDO_LISTCLIENT) == 0) {
                id_comando = ID_COMANDO_LISTCLIENT;
            } else if (strcmp(help_comando, COMANDO_EXIT) == 0) {
                id_comando = ID_COMANDO_EXIT;
            } else if (strcmp(help_comando, COMANDO_HELP) == 0) {
                id_comando = ID_COMANDO_HELP;
            } else {
                id_comando = atoi(help_comando);
            }
            menu_help(id_comando);
        } else {
            menu_help_comandos();   
        }
    }

    else
        printf("\nComando invalido!\n");

    printf("\nPressione qualquer tecla para continuar.\n");
    getchar();
    limpaTela();

    return 0;
}
