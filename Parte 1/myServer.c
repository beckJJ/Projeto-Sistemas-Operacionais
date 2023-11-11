#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 4000
#define NOME_DIRETORIO_SERVIDOR "sync_dir_SERVER"
#define MASCARA_PERMISSAO 0777
#define DIMENSAO_BUFFER 1024
#define DIMENSAO_GERAL 50

typedef struct
{
	char conteudo_arquivo[DIMENSAO_BUFFER];
	char nome_arquivo[DIMENSAO_GERAL];

} Pacote; 

void analisa_diretorio_servidor()
{
    struct stat st = {0};

    if (stat(NOME_DIRETORIO_SERVIDOR, &st) == -1)
    {
        if (!(mkdir(NOME_DIRETORIO_SERVIDOR, MASCARA_PERMISSAO) == 0))
        {
            printf("\nErro ao criar o diretorio do servidor!\n");
            getchar();
            exit(1);
	}
    }
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
		printf("\nErro! Nao foi possivel iniciar utilizacao do socket do servidor!\n");
		getchar();
		exit(1);       
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(socket_id, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		printf("\nErro! Nao foi possivel atribuir uma identidade ao socket do servidor!\n");
		getchar();
		exit(1);       
	}
	
	listen(socket_id, 5);
	
	clilen = sizeof(struct sockaddr_in);
	if ((newsocket_id = accept(socket_id, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
	{
		printf("\nErro! Nao foi possivel realizar conexao com o cliente!\n");
		getchar();
		exit(1);       
	}
	
	Pacote pacote;
	
	/* read from the socket */

	if (read(newsocket_id, pacote, sizeof(pacote)) < 0) 
	{
		printf("\nErro! Nao foi possivel realizar a leitura dos dados com o socket!\n");
		getchar();
		exit(1);       
	}
	
	printf("NOME DO ARQUIVO RECEBIDO:\n%s\n", pacote.nome_arquivo);
	
	close(newsocket_id);
	close(socket_id);
	return 0; 
}


