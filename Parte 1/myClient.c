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

#define DIMENSAO_GERAL 100
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50
#define DIMENSAO_NOME_DIRETORIO 200
#define DIMENSAO_BUFFER 1024

#define MASCARA_PERMISSAO 0777

#define REALIZOU_CONEXAO 1
#define FALHOU_CONEXAO 0

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

#define NOME_REQUISICAO_LISTSERVER "Requisicao_list_server"

#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

typedef struct
{
	char nome_usuario[DIMENSAO_NOME_USUARIO];
	char endereco_ip[DIMENSAO_GERAL];
	char numero_porta[DIMENSAO_GERAL];
	
	char comando[DIMENSAO_COMANDO];
	
	int socket_id;
	
} DadosConexao;


typedef struct
{
	char conteudo[DIMENSAO_BUFFER];
	char nome[DIMENSAO_GERAL];
	char usuario[DIMENSAO_NOME_USUARIO];
	
	int tamanho;
	int codigo_comando;

} Pacote; 

void limpa_tela()
{
    system("cls || clear");
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

    if (stat(nome_diretorio, &st) == -1)
    {
        if (!(mkdir(nome_diretorio, MASCARA_PERMISSAO) == 0))
        {
            printf("Erro ao criar o diretorio do cliente!\n");
            exit(EXIT_FAILURE);
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
            printf("Erro ao abrir o diretorio do cliente!\n");
            exit(EXIT_FAILURE);
    }
   
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
            printf("Erro ao obter informacoes do arquivo.\n");
            exit(EXIT_FAILURE);
        }
    }

    closedir(dir);
}

void list_client(DadosConexao dados_conexao)
{
    limpa_tela();
    
    char diretorioAtual[PATH_MAX];
    if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL) 
    {
        printf("Erro ao obter diretorio atual.\n");
        exit(EXIT_FAILURE);
    }

    char caminhoSyncDir[PATH_MAX];
    strcat(caminhoSyncDir,diretorioAtual);
    strcat(caminhoSyncDir,"/");
    strcat(caminhoSyncDir,PREFIXO_DIRETORIO);
    strcat(caminhoSyncDir,dados_conexao.nome_usuario);
    listarArquivosDiretorio(caminhoSyncDir);
}

void list_server(DadosConexao dados_conexao)
{
	limpa_tela();
	
	Pacote pacote;
	pacote.codigo_comando = CODIGO_LISTSERVER;
	strcpy(pacote.usuario,dados_conexao.nome_usuario);
	strcpy(pacote.nome,NOME_REQUISICAO_LISTSERVER);
	
	if (write(dados_conexao.socket_id, (void*)&pacote, sizeof(pacote)) < 0)
	{
		printf("Erro! Nao foi possivel mandar o pacote para requisitar os dados dos arquivos do cliente no servidor!\n");
		exit(EXIT_FAILURE);   	
	}
	 
	if (read(dados_conexao.socket_id, &pacote, sizeof(pacote)) < 0) 
	{
		printf("Erro! Nao foi possivel realizar a leitura dos dados com o socket, para visualizar os dados dos arquivos do cliente no servidor!\n");
		exit(EXIT_FAILURE);       
	}
	
	printf("ARQUIVOS SALVOS NO SERVIDOR, NO DIRETORIO DO USUARIO \n%s", dados_conexao.nome_usuario);
	printf("%s", pacote.conteudo);
	printf("\n------------------------------------------------------------------------------\n");
}

void obtemDiretorio(char *comando, char *diretorio) 
{
	int indice = 0;
	while (comando[indice] == ' ')
		indice++;
	while (comando[indice] != ' ')
		indice++;
	while (comando[indice] == ' ')
		indice++;
	int k = 0;
	for (int i = indice; i < strlen(comando); i++)
	{
		diretorio[k] = comando[i];
		k++;
	}
	diretorio[k] = '\0';
}

void obtemNomeArquivo(char *caminhoCompleto, char *nomeArquivo) 
{
	int conta_barras = 0, contador = 0;
	
	for (int i = 0; i < strlen(caminhoCompleto); i++)
		if (caminhoCompleto[i] == '\\' || caminhoCompleto[i] == '/')
			conta_barras++;
	
	int indice = 0;		
	for (indice = 0; contador != conta_barras; indice++)
		if (caminhoCompleto[indice] == '\\' || caminhoCompleto[indice] == '/')
			contador++;
			
	int k = 0;
	for (int i = indice; i < strlen(caminhoCompleto); i++)
	{
		nomeArquivo[k] = caminhoCompleto[i];
		k++;
	}
	nomeArquivo[k] = '\0';
}

void upload(DadosConexao dados_conexao)
{
	Pacote pacote;
	char diretorio[PATH_MAX];
	obtemDiretorio(dados_conexao.comando,diretorio);
	
	//printf("DIRETORIO:\n%s\n", diretorio);
	obtemNomeArquivo(diretorio,pacote.nome);
	//printf("NOME DO ARQUIVO:\n%s\n", pacote.nome);
	
	FILE *arquivo = fopen(diretorio, "rb");
	pacote.tamanho = fread(pacote.conteudo, sizeof(char), sizeof(pacote.conteudo), arquivo);
	fclose(arquivo);
	
	strcpy(pacote.usuario,dados_conexao.nome_usuario);
	pacote.codigo_comando = CODIGO_UPLOAD;
	
	if (write(dados_conexao.socket_id, (void*)&pacote, sizeof(pacote)) < 0)
	{
		printf("Erro! Nao foi possivel realizar a escrita no buffer!\n");
		exit(EXIT_FAILURE);   	
	}
}

int executa_comando(DadosConexao dados_conexao)
{
    if (strncmp(dados_conexao.comando,COMANDO_UPLOAD,strlen(COMANDO_UPLOAD)) == 0)
    {
    	if (verificaParametros(dados_conexao.comando,QUANTIDADE_PARAMETROS_UPLOAD) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		upload(dados_conexao);
    }

    else if (strncmp(dados_conexao.comando,COMANDO_DOWNLOAD,strlen(COMANDO_DOWNLOAD)) == 0)
    {
    	if (verificaParametros(dados_conexao.comando,QUANTIDADE_PARAMETROS_DOWNLOAD) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		printf("\nAcertou o comando!\n");
    }

    else if (strncmp(dados_conexao.comando,COMANDO_DELETE,strlen(COMANDO_DELETE)) == 0)
    {
    	if (verificaParametros(dados_conexao.comando,QUANTIDADE_PARAMETROS_DELETE) == 0)
    		printf("\nComando invalido! Numero de parametros incorreto!\n");
    	else
    		printf("\nAcertou o comando!\n");
    }

    else if (strcmp(dados_conexao.comando,COMANDO_LISTSERVER) == 0)
        list_server(dados_conexao);

    else if (strcmp(dados_conexao.comando,COMANDO_LISTCLIENT) == 0)
    	list_client(dados_conexao);

    else if (strcmp(dados_conexao.comando,COMANDO_EXIT) == 0)
    {
        limpa_tela();
        close(dados_conexao.socket_id);
        exit(0);
    }
    	
    else if (strcmp(dados_conexao.comando,COMANDO_HELP) == 0)
        menu_help();

    else
        printf("\nComando invalido!\n");
    
    printf("\nPressione qualquer tecla para continuar.\n");
    getchar();

}

void menu_principal(DadosConexao dados_conexao)
{
    printf("USUARIO CONECTADO:\n%s\n", dados_conexao.nome_usuario);
    printf("\nIP DO SERVIDOR:\n%s\n", dados_conexao.endereco_ip);
    printf("\nPORTA:\n%s\n", dados_conexao.numero_porta);
    printf("\nDigite 'help' para ver uma lista de comandos.\n");
    printf("Digite 'exit' para sair.\n");
    
    printf("\nDIGITE O COMANDO:\n");
}

int conecta_servidor(DadosConexao dados_conexao)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	server = gethostbyname(dados_conexao.endereco_ip);
	if (server == NULL) 
	{
		printf("Erro! Nenhum servidor com o IP %s foi encontrado!\n", dados_conexao.endereco_ip);
		exit(EXIT_FAILURE);
	}

	int socket_id;
	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		printf("Erro! Nao foi possivel iniciar utilizacao do socket do cliente!\n");
		exit(EXIT_FAILURE);       
	}

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(atoi(dados_conexao.numero_porta));    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     

	if (connect(socket_id,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
		printf("Erro! Nao foi possivel estabelecer conexao com o socket do servidor!\n");
		exit(EXIT_FAILURE);       
	}
	
	return socket_id;
}

int main (int argc, char *argv[])
{
    if (argc != QUANTIDADE_PARAMETROS_MYCLIENT+1)
    {
        printf("Comando invalido! Numero de parametros incorreto!\n");
        printf("Forma correta:\n./myClient <username> <server_ip_address> <port>\n");
        exit(EXIT_FAILURE);
    }
    
    DadosConexao dados_conexao;
    strcpy(dados_conexao.nome_usuario,argv[1]);
    strcpy(dados_conexao.endereco_ip,argv[2]);
    strcpy(dados_conexao.numero_porta,argv[3]);
    
    dados_conexao.socket_id = conecta_servidor(dados_conexao);
    analisa_diretorio(dados_conexao.nome_usuario);    
    
    while (1)
    {
        limpa_tela();
        menu_principal(dados_conexao);
        
        fgets(dados_conexao.comando,sizeof(dados_conexao.comando),stdin);
        dados_conexao.comando[strlen(dados_conexao.comando)-1] = '\0';

        executa_comando(dados_conexao);
    }
     
    limpa_tela();
    return 0;
}
