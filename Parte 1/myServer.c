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
#define NOME_DIRETORIO_SERVIDOR "sync_dir_SERVER"
#define MASCARA_PERMISSAO 0777
#define DIMENSAO_BUFFER 1024
#define DIMENSAO_GERAL 50
#define DIMENSAO_NOME_USUARIO 50

typedef struct
{
	char conteudo_arquivo[DIMENSAO_BUFFER];
	char nome_arquivo[DIMENSAO_GERAL];
	char usuario_arquivo[DIMENSAO_NOME_USUARIO];
	
	int tamanho;

} Pacote; 


void analisa_diretorio_servidor()
{
    struct stat st = {0};

    if (stat(NOME_DIRETORIO_SERVIDOR, &st) == -1)
    {
        if (!(mkdir(NOME_DIRETORIO_SERVIDOR, MASCARA_PERMISSAO) == 0))
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
            printf("Erro ao criar o diretorio do usuario %s, dentro do diretorio %s!\n",nomeNovoDiretorio,NOME_DIRETORIO_SERVIDOR);
            exit(EXIT_FAILURE);
        } 
    } 
}


void copiaArquivo(char *diretorioDestino, Pacote *pacote) 
{
    char caminhoCompleto[PATH_MAX];
    snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s/%s", diretorioDestino,pacote->usuario_arquivo, pacote->nome_arquivo);
    //printf("DIRETORIO PARA COPIAR ARQUIVO:\n%s\n", caminhoCompleto);

    FILE *novoArquivo = fopen(caminhoCompleto, "wb");
    if (novoArquivo == NULL) 
    {
        printf("Erro ao abrir o arquivo para escrita!\n");
        exit(EXIT_FAILURE);
    }

    size_t bytesEscritos = fwrite(pacote->conteudo_arquivo, sizeof(char), pacote->tamanho, novoArquivo);

    if (bytesEscritos != pacote->tamanho) 
    {
        printf("Erro ao escrever o conteúdo no novo arquivo!\n");
        fclose(novoArquivo);
        exit(EXIT_FAILURE);
    }

    fclose(novoArquivo);
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
	
	//printf("NOME DO ARQUIVO RECEBIDO:\n%s\n", pacote.nome_arquivo);
	//printf("CONTEUDO DO ARQUIVO RECEBIDO:\n%s\n", pacote.conteudo_arquivo);
	//printf("USUARIO QUE MANDOU O ARQUIVO RECEBIDO:\n%s\n", pacote.usuario_arquivo);
	
	criaNovoDiretorio(NOME_DIRETORIO_SERVIDOR, pacote.usuario_arquivo);
	copiaArquivo(NOME_DIRETORIO_SERVIDOR,&pacote);
	

	close(newsocket_id);
	close(socket_id);
	return 0; 
}
