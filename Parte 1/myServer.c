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

#define PORT 4000
#define PREFIXO_DIRETORIO_SERVIDOR "sync_dir_SERVER"
#define MASCARA_PERMISSAO 0777
#define DIMENSAO_BUFFER 1024
#define DIMENSAO_GERAL 100
#define DIMENSAO_NOME_USUARIO 50

#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

typedef struct
{
	char conteudo[DIMENSAO_BUFFER];
	char nome[DIMENSAO_GERAL];
	char usuario[DIMENSAO_NOME_USUARIO];
	
	int tamanho;
	int codigo_comando;

} Pacote; 


void analisa_diretorio_servidor()
{
    struct stat st = {0};

    if (stat(PREFIXO_DIRETORIO_SERVIDOR, &st) == -1)
    {
        if (!(mkdir(PREFIXO_DIRETORIO_SERVIDOR, MASCARA_PERMISSAO) == 0))
        {
            printf("Erro ao criar o diretorio do servidor!\n");
            exit(EXIT_FAILURE);
	}
    }
}

void criaNovoDiretorio(char *diretorioPai, char *nomeNovoDiretorio) 
{

    char caminhoCompleto[PATH_MAX];
    snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", diretorioPai, nomeNovoDiretorio);

    struct stat st = {0};
    if (stat(caminhoCompleto, &st) == -1) 
    {
        if (!(mkdir(caminhoCompleto, MASCARA_PERMISSAO) == 0))
        {
            printf("Erro ao criar o diretorio do usuario %s, dentro do diretorio %s!\n",nomeNovoDiretorio,PREFIXO_DIRETORIO_SERVIDOR);
            exit(EXIT_FAILURE);
        } 
    } 
}


void upload(char *diretorioDestino, Pacote *pacote) 
{
    char caminhoCompleto[PATH_MAX];
    snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s/%s", diretorioDestino,pacote->usuario, pacote->nome);
    //printf("DIRETORIO PARA COPIAR ARQUIVO:\n%s\n", caminhoCompleto);

    FILE *novoArquivo = fopen(caminhoCompleto, "wb");
    if (novoArquivo == NULL) 
    {
        printf("Erro ao abrir o arquivo para escrita!\n");
        exit(EXIT_FAILURE);
    }

    size_t bytesEscritos = fwrite(pacote->conteudo, sizeof(char), pacote->tamanho, novoArquivo);

    if (bytesEscritos != pacote->tamanho) 
    {
        printf("Erro ao escrever o conteúdo no novo arquivo!\n");
        fclose(novoArquivo);
        exit(EXIT_FAILURE);
    }

    fclose(novoArquivo);
}

void listarArquivosDiretorio(char *diretorio, Pacote *pacote)
{
    DIR *dir;
    struct dirent *entrada;
    struct stat info;

    dir = opendir(diretorio);
    if (dir == NULL) 
    {
            printf("Erro ao abrir o diretorio do servidor, com os arquivos do cliente!\n");
            exit(EXIT_FAILURE);
    }

    while ((entrada = readdir(dir)) != NULL)
    {
        char caminhoCompleto[PATH_MAX];
        snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", diretorio, entrada->d_name);

        if (stat(caminhoCompleto, &info) == 0) 
        {
            strcat(pacote->conteudo,"\nNOME:\n");
            strcat(pacote->conteudo,entrada->d_name);
            strcat(pacote->conteudo,"\nMODIFICATION TIME:\n");
            strcat(pacote->conteudo,ctime(&info.st_mtime));
            strcat(pacote->conteudo,"ACCESS TIME:\n");
            strcat(pacote->conteudo,ctime(&info.st_atime));
            strcat(pacote->conteudo,"CHANGE OR CREATION TIME:\n");
            strcat(pacote->conteudo,ctime(&info.st_ctime));
        } 
        
        else 
        {
            printf("Erro ao obter informacoes do arquivo.\n");
            exit(EXIT_FAILURE);
        }
    }

    closedir(dir);
}

void list_server(Pacote *pacote)
{
    char diretorioAtual[PATH_MAX];
    if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL) 
    {
        printf("Erro ao obter diretorio atual.\n");
        exit(EXIT_FAILURE);
    }

    char caminhoSyncDir[PATH_MAX];
    strcat(caminhoSyncDir,diretorioAtual);
    strcat(caminhoSyncDir,"/");
    strcat(caminhoSyncDir,PREFIXO_DIRETORIO_SERVIDOR);
    strcat(caminhoSyncDir,"/");
    strcat(caminhoSyncDir,pacote->usuario);
    listarArquivosDiretorio(caminhoSyncDir, pacote);   
}

void imprimeDadosPacote(Pacote pacote)
{
	time_t tempoAtual;
	struct tm *infoTempo;
	time(&tempoAtual);
	infoTempo = localtime(&tempoAtual);
	char buffer_tempo[DIMENSAO_GERAL];
	strftime(buffer_tempo, sizeof(buffer_tempo), "%Y-%m-%d %H:%M:%S", infoTempo);
	
	printf("\nNOME DO PACOTE RECEBIDO:\n%s\n", pacote.nome);
	printf("CLIENTE REMETENTE:\n%s\n", pacote.usuario);	
	printf("CODIGO DO COMANDO:\n%d\n", pacote.codigo_comando);
	printf("DATA E HORA DO RECEBIMENTO:\n%s\n", buffer_tempo);	
	printf("\n------------------------------------------------------------------------------\n");
}

int main(int argc, char *argv[])
{
	analisa_diretorio_servidor();
	
	int socket_id, newsocket_id, n;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	
	char buffer[DIMENSAO_BUFFER];
	
	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		printf("Erro! Nao foi possivel iniciar utilizacao do socket do servidor!\n");
		exit(EXIT_FAILURE);       
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(socket_id, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		printf("Erro! Nao foi possivel atribuir uma identidade ao socket do servidor!\n");
		exit(EXIT_FAILURE);       
	}
	
	listen(socket_id, 5);
	
	clilen = sizeof(struct sockaddr_in);
	if ((newsocket_id = accept(socket_id, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
	{
		printf("Erro! Nao foi possivel realizar conexao com o cliente!\n");
		exit(EXIT_FAILURE);       
	}
	
	Pacote pacote;
	

	if (read(newsocket_id, &pacote, sizeof(pacote)) < 0) 
	{
		printf("Erro! Nao foi possivel realizar a leitura dos dados com o socket!\n");
		exit(EXIT_FAILURE);       
	}
	
	imprimeDadosPacote(pacote);
	
	if (pacote.codigo_comando == CODIGO_UPLOAD)
	{
		criaNovoDiretorio(PREFIXO_DIRETORIO_SERVIDOR,pacote.usuario);
		upload(PREFIXO_DIRETORIO_SERVIDOR,&pacote);
	}
	else if (pacote.codigo_comando == CODIGO_LISTSERVER)
	{
		list_server(&pacote);
		
		if (write(newsocket_id, (void*)&pacote, sizeof(pacote)) < 0)
		{
			printf("Erro! Nao foi possivel realizar a escrita no buffer!\n");
			exit(EXIT_FAILURE);   	
		}
	}
	//else if (pacote.codigo_comado == CODIGO_DOWNLOAD)

	close(newsocket_id);
	close(socket_id);
	return 0; 
}
