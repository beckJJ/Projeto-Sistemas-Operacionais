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

#include "comunicacaoServidor.hpp"

/* Envia um pacote contendo um arquivo, que foi solicitado por um cliente. */
void download(int newsocket_id, Pacote *pacote)
{
	/* Obtem diretorio atual, para acessar o diretorio sync_dir_SERVER */
	char diretorioAtual[PATH_MAX];
	if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL)
	{
		printf("Erro ao obter diretorio atual.\n");
		exit(EXIT_FAILURE);
	}

	/* Identifica, no nome do pacote, a operacao que o envolve. */
	strcpy(pacote->nomePacote, NOME_RESPOSTA_DOWNLOAD);

	/* Obtem o caminho contendo o repositorio remoto do cliente que requisitou o download. */
	char caminhoSyncDir[PATH_MAX];
	memset(caminhoSyncDir, 0, sizeof(caminhoSyncDir));
	strcat(caminhoSyncDir, diretorioAtual);
	strcat(caminhoSyncDir, "/");
	strcat(caminhoSyncDir, PREFIXO_DIRETORIO_SERVIDOR);
	strcat(caminhoSyncDir, "/");
	strcat(caminhoSyncDir, pacote->usuario);
	strcat(caminhoSyncDir, "/");
	strcat(caminhoSyncDir, pacote->nomeArquivo);

	// printf("DIRETORIO DO ARQUIVO SOLICITADO:\n%s\n", caminhoSyncDir);

	/* Abre o arquivo solicitado pelo cliente, para copiar suas informacoes em um buffer. */
	FILE *arquivo;
	if (!(arquivo = fopen(caminhoSyncDir, "rb")))
	{
		pacote->codigoComunicacao = CODIGO_ERRO_DOWNLOAD;

		if (write(newsocket_id, (void *)pacote, sizeof(*pacote)) < 0)
		{
			printf("Erro! Nao foi possivel realizar a escrita no buffer para realizar o download!\n");
			exit(EXIT_FAILURE);
		}
		return;
	}

	/* Copia todas as informacoes do arquivo solicitado, em um pacote a ser enviado ao cliente. */
	pacote->tamanho = fread(pacote->conteudo, sizeof(char), sizeof(pacote->conteudo), arquivo);
	// printf("CONTEUDO RECEBIDO NO PACOTE:\n%s\n", pacote->conteudo);
	fclose(arquivo);

	/* Realiza o envio do pacote contendo o arquivo para o cliente solicitante. */
	if (write(newsocket_id, (void *)pacote, sizeof(*pacote)) < 0)
	{
		printf("Erro! Nao foi possivel realizar a escrita no buffer para realizar o download!\n");
		exit(EXIT_FAILURE);
	}
}

/* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente em um pacote. */
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

	/* Iteracao para percorrer o diretorio e listar todos os arquivos, junto aos seus MAC times. */
	while ((entrada = readdir(dir)) != NULL)
	{
		char caminhoCompleto[PATH_MAX];
		snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", diretorio, entrada->d_name);

		if (stat(caminhoCompleto, &info) == 0)
		{
			strcat(pacote->conteudo, "\nNOME:\n");
			strcat(pacote->conteudo, entrada->d_name);
			strcat(pacote->conteudo, "\nMODIFICATION TIME:\n");
			strcat(pacote->conteudo, ctime(&info.st_mtime));
			strcat(pacote->conteudo, "ACCESS TIME:\n");
			strcat(pacote->conteudo, ctime(&info.st_atime));
			strcat(pacote->conteudo, "CHANGE OR CREATION TIME:\n");
			strcat(pacote->conteudo, ctime(&info.st_ctime));
		}

		else
		{
			printf("Erro ao obter informacoes do arquivo.\n");
			exit(EXIT_FAILURE);
		}
	}

	closedir(dir);
}

/* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente em um pacote, e envia o pacote ao cliente solicitante. */
void list_server(int newsocket_id, Pacote *pacote)
{
	/* Identifica, no nome do pacote, a operacao que o envolve. */
	strcpy(pacote->nomePacote, NOME_RESPOSTA_LISTSERVER);

	char diretorioAtual[PATH_MAX];
	if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL)
	{
		printf("Erro ao obter diretorio atual.\n");
		exit(EXIT_FAILURE);
	}

	/* Armazena as informacoes do diretorio a ser analisado, em uma string. */
	char caminhoSyncDir[PATH_MAX];
	memset(caminhoSyncDir, 0, sizeof(caminhoSyncDir));
	strcat(caminhoSyncDir, diretorioAtual);
	strcat(caminhoSyncDir, "/");
	strcat(caminhoSyncDir, PREFIXO_DIRETORIO_SERVIDOR);
	strcat(caminhoSyncDir, "/");
	strcat(caminhoSyncDir, pacote->usuario);
	listarArquivosDiretorio(caminhoSyncDir, pacote); /* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente no pacote. */

	/* Escreve as informacoes sobre o repositorio remoto no pacote, e envia o pacote ao cliente. */
	if (write(newsocket_id, (void *)pacote, sizeof(*pacote)) < 0)
	{
		printf("Erro! Nao foi possivel realizar a escrita no buffer para listar os arquivos!\n");
		exit(EXIT_FAILURE);
	}
}

/* Armazena um arquivo enviado pelo cliente, no repositorio remoto associado a ele. */
void upload(const char *diretorioDestino, Pacote *pacote)
{
	char caminhoCompleto[PATH_MAX];
	snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s/%s", diretorioDestino, pacote->usuario, pacote->nomeArquivo);
	// printf("DIRETORIO PARA COPIAR ARQUIVO:\n%s\n", caminhoCompleto);

	/* Cria um arquivo, que ira conter os dados enviados no pacote, do arquivo a ser armazenado no servidor. */
	FILE *novoArquivo = fopen(caminhoCompleto, "wb");
	if (novoArquivo == NULL)
	{
		printf("Erro ao abrir o novo arquivo para escrita!\n");
		exit(EXIT_FAILURE);
	}

	/* Realiza a escrita dos dados no novo arquivo criado no repositorio, controlando se o numero de bytes lidos esta correto. */
	size_t bytesEscritos = fwrite(pacote->conteudo, sizeof(char), pacote->tamanho, novoArquivo);
	if (bytesEscritos != pacote->tamanho)
	{
		printf("Erro ao escrever o conteúdo no novo arquivo!\n");
		fclose(novoArquivo);
		exit(EXIT_FAILURE);
	}

	fclose(novoArquivo);
}

/* Imprime, no terminal, os dados relacionados com um pacote que foi recebido pelo servidor ou enviado pelo servidor. */
void imprimeDadosPacote(Pacote pacote)
{
	time_t tempoAtual;
	struct tm *infoTempo;
	time(&tempoAtual);
	infoTempo = localtime(&tempoAtual);
	char buffer_tempo[DIMENSAO_GERAL];
	strftime(buffer_tempo, sizeof(buffer_tempo), "%Y-%m-%d %H:%M:%S", infoTempo);

	printf("\nNOME DO PACOTE:\n%s\n", pacote.nomePacote);
	printf("CLIENTE:\n%s\n", pacote.usuario);
	printf("CODIGO DA INTERACAO:\n%d\n", pacote.codigoComunicacao);
	printf("DATA E HORA DA INTERACAO:\n%s\n", buffer_tempo);
	printf("\n------------------------------------------------------------------------------\n");
}
