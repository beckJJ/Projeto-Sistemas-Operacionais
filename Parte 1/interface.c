#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define ESC 27
#define DIMENSAO_COMANDO 200
#define MASCARA_PERMISSAO 0777

#define COMANDO_MYCLIENT "myClient"
#define COMANDO_UPLOAD "upload"
#define COMANDO_DOWNLOAD "download"
#define COMANDO_DELETE "delete"
#define COMANDO_LISTSERVER "list_server"
#define COMANDO_LISTCLIENT "list_client"
#define COMANDO_EXIT "exit"
#define COMANDO_HELP "help"

#include <stdlib.h>
#include <stdio.h>

void limpa_tela()
{
    system("clear || cls");
}

void menu_help() /* Funcao para realizar a impressao do menu de ajuda do programa na tela. */
{
    system("clear || cls");
    printf("\nLISTA DE COMANDOS:\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 1:");
    printf("\n\nSINTAXE:\nmyClient <username> <server_ip_address> <port>");
    printf("\n\nDESCRICAO:\n");
    printf("Estabelece conexao entre cliente e servidor.\n");
    printf("<username> representa o identificador do usuario, e <server_ip_address>\n");
    printf("e <port> representam o endereco IP do servidor e a porta, respectivamente\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 2:");
    printf("\n\nSINTAXE:\nupload <path/filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Envia o arquivo filename.ext para o servidor, colocando-o no ""sync_dir"" do\n");
    printf("servidor e propagando-o para todos os dispositivos do usuario.\n");
    printf("e.g. upload /home/user/MyFolder/filename.ext\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 3:");
    printf("\n\nSINTAXE:\ndownload <filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Faz uma copia nao sincronizada do arquivo filename.ext do servidor para\n");
    printf("o diretorio local (de onde o servidor foi chamado).\n");
    printf("e.g. download mySpreadsheet.xlsx\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 4:");
    printf("\n\nSINTAXE:\ndelete <filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Exclui o arquivo <filename.ext> de ""sync_dir"".\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 5:");
    printf("\n\nSINTAXE:\nlist_server");
    printf("\n\nDESCRICAO:\n");
    printf("Lista os arquivos salvos no servidor, associados ao usuario.\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 6:");
    printf("\n\nSINTAXE:\nlist_client");
    printf("\n\nDESCRICAO:\n");
    printf("Lista os arquivos salvos no diretorio ""sync_dir"", do dispositivo local do usuario.\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 7:");
    printf("\n\nSINTAXE:\nexit");
    printf("\n\nDESCRICAO:\n");
    printf("Fecha a sessao com o servidor.\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nPressione ESC para encerrar a execucao do programa, ou qualquer outra tecla para voltar ao menu inicial.\n");
}

int executa_comando(char *comando)
{
    if (strncmp(comando,COMANDO_MYCLIENT,strlen(COMANDO_MYCLIENT)) == 0)
        puts(COMANDO_MYCLIENT);

    else if (strncmp(comando,COMANDO_UPLOAD,strlen(COMANDO_UPLOAD)) == 0)
        puts(COMANDO_UPLOAD);

    else if (strncmp(comando,COMANDO_DOWNLOAD,strlen(COMANDO_DOWNLOAD)) == 0)
        puts(COMANDO_DOWNLOAD);

    else if (strncmp(comando,COMANDO_DELETE,strlen(COMANDO_DELETE)) == 0)
        puts(COMANDO_DELETE);

    else if (strcmp(comando,COMANDO_LISTSERVER) == 0)
        puts(COMANDO_LISTSERVER);

    else if (strcmp(comando,COMANDO_LISTCLIENT) == 0)
        puts(COMANDO_LISTCLIENT);

    else if (strcmp(comando,COMANDO_EXIT) == 0)
        puts(COMANDO_EXIT);

    else if (strcmp(comando,COMANDO_HELP) == 0)
        menu_help();

    else
        printf("\nComando invalido!\n");
}

void menu_principal()
{
    printf("STATUS: SEM CONEXAO COM SERVIDOR\n");
    printf("PORTA: --\t IP: --\n");
    printf("\nDigite o comando:\n");
}

int main ()
{
    char comando[DIMENSAO_COMANDO];
    char tecla;
    int codigo_comando;

    do
    {
        limpa_tela();
        menu_principal();
        gets(comando);

        codigo_comando = executa_comando(comando);
        tecla = getch();

    } while (tecla != ESC);

    limpa_tela();

    return 0;
}
