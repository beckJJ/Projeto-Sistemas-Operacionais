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

#include "comunicacaoServidor.h"
#include "auxiliaresServidor.h"

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
		
	while(1)
	{

		if (read(newsocket_id, &pacote, sizeof(pacote)) < 0) 
		{
			printf("Erro! Nao foi possivel realizar a leitura dos dados com o socket!\n");
			exit(EXIT_FAILURE);       
		}
		
		printf("\n<<< PACOTE RECEBIDO >>>\n");
		imprimeDadosPacote(pacote);
		
		if (pacote.codigoComunicacao == CODIGO_UPLOAD)
		{
			criaNovoDiretorio(PREFIXO_DIRETORIO_SERVIDOR,pacote.usuario);
			upload(PREFIXO_DIRETORIO_SERVIDOR,&pacote);
		}
		else if (pacote.codigoComunicacao == CODIGO_LISTSERVER)
		{
			list_server(newsocket_id, &pacote);
			printf("\n<<< PACOTE ENVIADO >>>\n");
			imprimeDadosPacote(pacote);
		}
		else if (pacote.codigoComunicacao == CODIGO_DOWNLOAD)
		{
			download(newsocket_id, &pacote);
			printf("\n<<< PACOTE ENVIADO >>>\n");
			imprimeDadosPacote(pacote);
		}
	}
	
	close(newsocket_id);
	close(socket_id);
	return 0; 
}
