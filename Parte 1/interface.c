#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define ESC 27

#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50
#define DIMENSAO_NOME_DIRETORIO 200

#define MASCARA_PERMISSAO 0777

#define REALIZOU_CONEXAO 1
#define FALHOU_CONEXAO 0

#define NOME_DIRETORIO_SERVIDOR "sync_dir_servidor"
#define PREFIXO_DIRETORIO_USUARIO "sync_dir_"

#define COMANDO_MYCLIENT "myClient"
#define COMANDO_UPLOAD "upload"
#define COMANDO_DOWNLOAD "download"
#define COMANDO_DELETE "delete"
#define COMANDO_LISTSERVER "list_server"
#define COMANDO_LISTCLIENT "list_client"
#define COMANDO_EXIT "exit"
#define COMANDO_HELP "help"

#define QUANTIDADE_PARAMETROS_MYCLIENT 3
#define QUANTIDADE_PARAMETROS_UPLOAD 1
#define QUANTIDADE_PARAMETROS_DOWNLOAD 1
#define QUANTIDADE_PARAMETROS_DELETE 1

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
    printf("\n\nSINTAXE:\nupload <path/filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Envia o arquivo filename.ext para o servidor, colocando-o no ""sync_dir"" do\n");
    printf("servidor e propagando-o para todos os dispositivos do usuario.\n");
    printf("e.g. upload /home/user/MyFolder/filename.ext\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 2:");
    printf("\n\nSINTAXE:\ndownload <filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Faz uma copia nao sincronizada do arquivo filename.ext do servidor para\n");
    printf("o diretorio local (de onde o servidor foi chamado).\n");
    printf("e.g. download mySpreadsheet.xlsx\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 3:");
    printf("\n\nSINTAXE:\ndelete <filename.ext>");
    printf("\n\nDESCRICAO:\n");
    printf("Exclui o arquivo <filename.ext> de ""sync_dir"".\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 4:");
    printf("\n\nSINTAXE:\nlist_server");
    printf("\n\nDESCRICAO:\n");
    printf("Lista os arquivos salvos no servidor, associados ao usuario.\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 5:");
    printf("\n\nSINTAXE:\nlist_client");
    printf("\n\nDESCRICAO:\n");
    printf("Lista os arquivos salvos no diretorio ""sync_dir"", do dispositivo local do usuario.\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 6:");
    printf("\n\nSINTAXE:\nexit");
    printf("\n\nDESCRICAO:\n");
    printf("Fecha a sessao com o servidor.\n");

    printf("\n----------------------------------------------------------------------------------------------------\n");

    printf("\nPressione ESC para encerrar a execucao do programa, ou qualquer outra tecla para voltar ao menu inicial.\n");
}

int verificaParametros(char *comando, int quantidade_parametros)
{
    int numPalavras = 0;
    int dentroPalavra = 0;
    for (int i = 0; comando[i] != '\0'; i++)
    {
        if (comando[i] == ' ' || comando[i] == '\t')
            dentroPalavra = 0;

        else
        {
            if (dentroPalavra == 0)
            {
                numPalavras++;
                dentroPalavra = 1;
            }
        }
    }
    return (numPalavras == quantidade_parametros+1);
}

void obtemNomeUsuario(char *comando, char *nome_usuario)
{
    int contador = 0;
    while (comando[contador] == ' ')
        contador++;
    while(comando[contador] != ' ')
        contador++;
    while (comando[contador] == ' ')
        contador++;

    int i;
    for (i = 0; comando[contador+i] != ' ' && comando[contador+i] != '\t'; i++)
        nome_usuario[i] = comando[contador+i];
    nome_usuario[i] = '\0';

}

void analisa_diretorio(char *nome_usuario)
{
    sprintf(nome_diretorio,PREFIXO_DIRETORIO_USUARIO);
    strcat(nome_diretorio,nome_usuario);

    struct stat st = {0};

    if (stat(nome_diretorio, &st) == -1)
    {
        if (mkdir(nome_diretorio) == 0)
            printf("\nDiretorio %s CRIADO com sucesso!\n", nome_diretorio);

        else
            printf("\nErro ao criar o diretorio!\n");
    }

    else
        printf("\nDiretorio %s CARREGADO com sucesso!\n", nome_diretorio);
}

int executa_comando(char *comando)
{
    if (strncmp(comando,COMANDO_MYCLIENT,strlen(COMANDO_MYCLIENT)) == 0)
    {
        analisa_diretorio(comando);
    }

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
    {
        printf("\nComando invalido!\n");
        printf("\nPressione ESC para encerrar a execucao do programa, ou qualquer outra tecla para voltar ao menu inicial.\n");
    }
}

void menu_principal()
{
    printf("STATUS: SEM CONEXAO COM SERVIDOR\n");
    printf("PORTA: --\t IP: --\n");
    printf("\nDigite o comando:\n");
}

int main (int argc, char arg[])
{
    if (argc < QUANTIDADE_PARAMETROS_MYCLIENT)
    {
        printf("\nComando invalido! Numero de parametros incorreto!\n");
        printf("Forma correta:\n./myClient <nome de usuario> <IP do servidor> <numero de porta>\n")
    }

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
