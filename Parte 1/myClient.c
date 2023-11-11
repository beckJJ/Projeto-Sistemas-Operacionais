#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>

#define DIMENSAO_GERAL 50
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50
#define DIMENSAO_NOME_DIRETORIO 200

#define MASCARA_PERMISSAO 0777

#define REALIZOU_CONEXAO 1
#define FALHOU_CONEXAO 0

#define NOME_DIRETORIO_SERVIDOR "sync_dir_servidor"
#define PREFIXO_DIRETORIO "sync_dir_"

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

typedef struct
{
	char nome_usuario[DIMENSAO_NOME_USUARIO];
	char endereco_ip[DIMENSAO_GERAL];
	char numero_porta[DIMENSAO_GERAL];
	
} DadosConexao;

void limpa_tela()
{
    system("clear || cls");
}

void menu_help()
{
    limpa_tela();
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
    printf("Lista os arquivos salvos no diretorio 'sync_dir', do dispositivo local do usuario.\n");

    printf("\n------------------------------------------------------------------------------\n");

    printf("\nCOMANDO 6:");
    printf("\n\nSINTAXE:\nexit");
    printf("\n\nDESCRICAO:\n");
    printf("Fecha a sessao com o servidor.\n");

    printf("\n------------------------------------------------------------------------------\n");
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

void analisa_diretorio(char *nome_usuario)
{
    char nome_diretorio[DIMENSAO_NOME_DIRETORIO];
    sprintf(nome_diretorio,PREFIXO_DIRETORIO);
    strcat(nome_diretorio,nome_usuario);

    struct stat st = {0};
    
    printf("%s\n", nome_diretorio);

    if (stat(nome_diretorio, &st) == -1)
    {
        if (!(mkdir(nome_diretorio, MASCARA_PERMISSAO) == 0))
        {
            printf("\nErro ao criar o diretorio!\n");
            getchar();
            exit(1);
	}
    }
}

void listarArquivosDiretorio(char *diretorio) 
{
    DIR *dir;
    struct dirent *entrada;
    struct stat info;

    dir = opendir(diretorio);
    if (dir == NULL) 
    {
            printf("\nErro ao abrir o diretorio.\n");
            exit(1);
    }
    
    limpa_tela();
    printf("ARQUIVOS DO DIRETORIO\n%s\n", diretorio);

    while ((entrada = readdir(dir)) != NULL)
    {
        char caminhoCompleto[PATH_MAX];
        snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", diretorio, entrada->d_name);

        if (stat(caminhoCompleto, &info) == 0) 
        {
            printf("\nNOME:\n%s\n", entrada->d_name);
            printf("MODIFICATION TIME:\n%s", ctime(&info.st_mtime));
            printf("ACCESS TIME:\n%s", ctime(&info.st_atime));
            printf("CHANGE OR CREATION TIME:\n%s", ctime(&info.st_ctime));
            printf("\n------------------------------------------------------------------------------\n");
        } 
        
        else 
        {
            printf("\nErro ao obter informacoes do arquivo.\n");
            exit(1);
        }
    }

    closedir(dir);
}

void list_client(DadosConexao dados_conexao)
{
    char diretorioAtual[PATH_MAX];
    if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL) 
    {
        printf("\nErro ao obter diretorio atual.\n");
        exit(1);
    }

    char caminhoSyncDir[PATH_MAX];
    //snprintf(caminhoSyncDir, sizeof(caminhoSyncDir), "%s/sync_dir", diretorioAtual);
    strcat(caminhoSyncDir,diretorioAtual);
    strcat(caminhoSyncDir,"/");
    strcat(caminhoSyncDir,PREFIXO_DIRETORIO);
    strcat(caminhoSyncDir,dados_conexao.nome_usuario);
    //printf("\nDIRETORIO:%s\n", caminhoSyncDir);
    listarArquivosDiretorio(caminhoSyncDir);
}

int executa_comando(char *comando, DadosConexao dados_conexao)
{
    if (strncmp(comando,COMANDO_UPLOAD,strlen(COMANDO_UPLOAD)) == 0)
    {
    	if (verificaParametros(comando,QUANTIDADE_PARAMETROS_UPLOAD) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		printf("\nAcertou o comando!\n");
    }

    else if (strncmp(comando,COMANDO_DOWNLOAD,strlen(COMANDO_DOWNLOAD)) == 0)
    {
    	if (verificaParametros(comando,QUANTIDADE_PARAMETROS_DOWNLOAD) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		printf("\nAcertou o comando!\n");
    }

    else if (strncmp(comando,COMANDO_DELETE,strlen(COMANDO_DELETE)) == 0)
    {
    	if (verificaParametros(comando,QUANTIDADE_PARAMETROS_DELETE) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		printf("\nAcertou o comando!\n");
    }

    else if (strcmp(comando,COMANDO_LISTSERVER) == 0)
        puts(COMANDO_LISTSERVER);

    else if (strcmp(comando,COMANDO_LISTCLIENT) == 0)
    	list_client(dados_conexao);

    else if (strcmp(comando,COMANDO_EXIT) == 0)
    {
        limpa_tela();
        exit(0);
    }
    	
    else if (strcmp(comando,COMANDO_HELP) == 0)
        menu_help();

    else
        printf("\nComando invalido!\n");
    
    printf("\nPressione qualquer tecla para continuar.\n");

}

void menu_principal(DadosConexao dados_conexao)
{
    printf("USUARIO ATUAL:\n%s\n", dados_conexao.nome_usuario);
    printf("\nIP DO SERVIDOR:\n%s\n", dados_conexao.endereco_ip);
    printf("\nPORTA:\n%s\n", dados_conexao.numero_porta);
    printf("\nDigite 'exit' para sair.\n");
    printf("Digite 'help' para ver uma lista de comandos.\n");
    
    printf("\nDIGITE O COMANDO:\n");
}

int main (int argc, char *argv[])
{
    if (argc != QUANTIDADE_PARAMETROS_MYCLIENT+1)
    {
        printf("Comando invalido! Numero de parametros incorreto!\n");
        printf("Forma correta:\n./myClient <username> <server_ip_address> <port>\n");
        exit(1);
    }
    
    DadosConexao dados_conexao;
    strcpy(dados_conexao.nome_usuario,argv[1]);
    strcpy(dados_conexao.endereco_ip,argv[2]);
    strcpy(dados_conexao.numero_porta,argv[3]);
    analisa_diretorio(dados_conexao.nome_usuario);    
    char comando[DIMENSAO_COMANDO];

    while (1)
    {
        limpa_tela();
        menu_principal(dados_conexao);
        
        fgets(comando,sizeof(comando),stdin);
        comando[strlen(comando)-1] = '\0';

        executa_comando(comando, dados_conexao);
        getchar();

    }
     
    limpa_tela();
    return 0;
}
