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
	DeviceManager deviceMan;
	analisa_diretorio_servidor();

	uint16_t port = PORT;
	char c;

	while ((c = getopt(argc, argv, "hp:")) != -1)
	{
		switch (c)
		{
		case 'h':
			puts("Opcoes:");
			puts("\t-p PORT\tPorta que devera ser usada pelo servidor.");
			puts("\t-h\tExibe mensagem de ajuda.");
			exit(0);
		case 'p':
			port = atoi(optarg);
			break;
		default:
			printf("Usage:\n");
			printf("\t%s [-p PORT]\n", argv[0]);
			abort();
		}
	}

	int socket_id;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Erro! Nao foi possivel iniciar utilizacao do socket do servidor!\n");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), sizeof(serv_addr.sin_zero));

	if (bind(socket_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Erro! Nao foi possivel atribuir uma identidade ao socket do servidor!\n");
		exit(EXIT_FAILURE);
	}

	listen(socket_id, 5);

	printf("Servidor está escutando na porta %d.\n", port);

	clilen = sizeof(struct sockaddr_in);

	while (1)
	{
		thread_arg_t thread_arg { };
		thread_arg.deviceMan = &deviceMan;

		if ((thread_arg.socket = accept(socket_id, (struct sockaddr *)&cli_addr, &clilen)) == -1)
		{
			printf("Erro! Nao foi possivel realizar conexao com o cliente!\n");
			exit(EXIT_FAILURE);
		}

		printf("Nova conexao estabelecida.\n");

		pthread_create(&thread_arg.thread, NULL, servFunc, &thread_arg);
	}

	// Cancela todas as threads que ainda estejam executando
	deviceMan.disconnect_all();

	close(socket_id);

	return 0;
}
