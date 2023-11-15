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
#include <pthread.h>
#include <vector>

#include "comunicacaoServidor.hpp"
#include "auxiliaresServidor.hpp"
#include "servFunc.hpp"

int main(int argc, char *argv[])
{
	std::vector<pthread_t> threads;
	analisa_diretorio_servidor();

	int socket_id;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Erro! Nao foi possivel iniciar utilizacao do socket do servidor!\n");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(socket_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Erro! Nao foi possivel atribuir uma identidade ao socket do servidor!\n");
		exit(EXIT_FAILURE);
	}

	listen(socket_id, 5);

	clilen = sizeof(struct sockaddr_in);

	while (1)
	{
		pthread_t thread;
		int new_socket;

		if ((new_socket = accept(socket_id, (struct sockaddr *)&cli_addr, &clilen)) == -1)
		{
			printf("Erro! Nao foi possivel realizar conexao com o cliente!\n");
			exit(EXIT_FAILURE);
		}

		printf("Nova conexao estabelecida.\n");

		threads.push_back(thread);
		pthread_create(&thread, NULL, servFunc, &new_socket);
	}

	close(socket_id);

	printf("Closing server, all living threads will be cancelled.\n");

	// Closes all threads
	for (auto thread: threads) {
		pthread_cancel(thread);
	}

	return 0;
}
