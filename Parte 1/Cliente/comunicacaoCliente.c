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

#include "comunicacaoCliente.h"
#include "interfaceCliente.h"
#include "auxiliaresCliente.h"

int conecta_servidor(DadosConexao dados_conexao) /* Inicia a conexao com o servidor, via socket. */
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	server = gethostbyname(dados_conexao.endereco_ip); /* Obtem informacao do host, a partir do seu endereco IP. */
	if (server == NULL) 
	{
		printf("Erro! Nenhum servidor com o IP %s foi encontrado!\n", dados_conexao.endereco_ip);
		exit(EXIT_FAILURE);
	}

	int socket_id;
	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) /* Inicia a utilizacao de um socket, guardando o valor inteiro que o referencia. */
	{
		printf("Erro! Nao foi possivel iniciar utilizacao do socket do cliente!\n");
		exit(EXIT_FAILURE);  
	}

	serv_addr.sin_family = AF_INET; /* Identifica o protocolo de rede a ser utilizado. */   
	serv_addr.sin_port = htons(atoi(dados_conexao.numero_porta)); /* Identifica o numero de porta a ser utilizado. */ 
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     

	if (connect(socket_id,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  /* Solicita abertura de conexao com o servidor. */
	{
		printf("Erro! Nao foi possivel estabelecer conexao com o socket do servidor!\n");
		exit(EXIT_FAILURE);
	}
	
	return socket_id; /* Retorna o identificador do socket utilizado para estabelecer conexao com servidor. */
}

void upload(DadosConexao dados_conexao) /* Realiza o envio de um arquivo para o diretorio do usuario, presente no servidor. */
{
	Pacote pacote;
	inicializaPacote(&pacote);
	pacote.codigoComunicacao = CODIGO_UPLOAD; /* Identifica, no pacote, o codigo da operacao a ser realizada com o servidor. */
	strcpy(pacote.usuario,dados_conexao.nome_usuario);
	strcpy(pacote.nomePacote,NOME_REQUISICAO_UPLOAD);
	
	char diretorio[PATH_MAX];
	obtemDiretorioComando(dados_conexao.comando,diretorio); /* Recebe um comando digitado pelo usuario, e obtem o nome do diretorio referenciado no comando. */
	
	//printf("DIRETORIO:\n%s\n", diretorio);
	obtemNomeArquivoDiretorio(diretorio,pacote.nomeArquivo); /* Recebe um diretorio escolhido pelo usuario, e obtem o nome do arquivo final referenciado no diretorio. */
	//printf("NOME DO ARQUIVO:\n%s\n", pacote.nomeArquivo);
	
	FILE *arquivo;
	/* Realiza a abertura do arquivo local a ser enviado, para escrever seu conteudo em um buffer. */
	if (!(arquivo = fopen(diretorio, "rb")))
	{
		printf("Erro! Arquivo '%s' nao encontrado no diretorio 'sync_dir_%s'\n", pacote.nomeArquivo, dados_conexao.nome_usuario);
		return;		
	}
	
	/* Faz a leitura do conteudo do arquivo que foi aberto, e escreve o conteudo no pacote a ser enviado ao servidor. */
	pacote.tamanho = fread(pacote.conteudo, sizeof(char), sizeof(pacote.conteudo), arquivo);
	fclose(arquivo);
	
	if (write(dados_conexao.socket_id, (void*)&pacote, sizeof(pacote)) < 0) /* Faz a escrita dos dados no pacote a ser enviado ao servidor. */
	{
		printf("Erro! Nao foi possivel realizar a escrita no buffer!\n");
		return;	
	}
	
    	printf("\nArquivo '%s' enviado com sucesso.\n", pacote.nomeArquivo);
}

/* Recebe um arquivo do servidor, e copia para o diretorio de onde a aplicacao cliente foi chamada. */
void download (DadosConexao dados_conexao) 
{
	char nomeArquivo[DIMENSAO_GERAL];
	obtemNomeArquivoComando(dados_conexao.comando, nomeArquivo); /* Recebe um comando digitado pelo usuario, e obtem o nome do arquivo final referenciado no comando. */
	
	/* Inicializa e copia os dados da comunicacao no pacote. */
	Pacote pacote;
	inicializaPacote(&pacote);
	pacote.codigoComunicacao = CODIGO_DOWNLOAD;
	strcpy(pacote.usuario,dados_conexao.nome_usuario);
	strcpy(pacote.nomePacote,NOME_REQUISICAO_DOWNLOAD);
	strcpy(pacote.nomeArquivo,nomeArquivo);
	
	/* Escreve os dados no pacote de REQUISICAO a ser enviado ao servidor, para pedir por um arquivo. */
	if (write(dados_conexao.socket_id, (void*)&pacote, sizeof(pacote)) < 0)
	{
		printf("Erro! Nao foi possivel mandar o pacote para requisitar os dados dos arquivos do cliente no servidor!\n");
		return;
	}
	 
	/* Faz a leitura dos dados do arquivo recebido pelo servidor. */
	if (read(dados_conexao.socket_id, &pacote, sizeof(pacote)) < 0) 
	{
		printf("Erro! Nao foi possivel realizar a leitura dos dados com o socket, para visualizar os dados dos arquivos do cliente no servidor!\n");
		return;   
	}
	
	/* Verifica se houve erro no recebimento do arquivo enviado pelo servidor. */
	if (pacote.codigoComunicacao == CODIGO_ERRO_DOWNLOAD)
	{
		printf("Erro! Arquivo '%s' nao encontrado no diretorio do servidor!\n", nomeArquivo);
		return;
	}
	
	/* Cria um arquivo na maquina local, para copiar o conteudo recebido no pacote enviado pelo servidor. */
	FILE *novoArquivo;
	if (!(novoArquivo = fopen(nomeArquivo, "wb")))
	{
		printf("Erro ao criar um arquivo na maquina local, para receber os dados do arquivo baixado!\n");
		return;	
	}
	
	/* Copia o conteudo recebido pelo servidor no novo arquivo da maquina local. */
	size_t bytesEscritos = fwrite(pacote.conteudo, sizeof(char), pacote.tamanho, novoArquivo);
	fclose(novoArquivo); 

	if (bytesEscritos != pacote.tamanho) /* Verifica se a quantidade de bytes copiada esta correta. */
	{
		printf("Erro ao escrever o conteúdo no novo arquivo!\n");
		return;
	}
	
	printf("\nArquivo '%s' baixado do servidor com sucesso.\n", nomeArquivo);
}

void delete(DadosConexao dados_conexao) /* Delete um arquivo presente na maquina local do usuario, no diretorio sync_dir_<usuario> */
{
	//printf("COMANDO:\n%s\n", dados_conexao.comando);
	
	char nomeArquivo[DIMENSAO_GERAL];
	obtemNomeArquivoComando(dados_conexao.comando,nomeArquivo); /* Recebe um comando digitado pelo usuario, e obtem o nome do arquivo final referenciado no comando. */

	//printf("NOME DO ARQUIVO:\n%s\n", nomeArquivo);

	char diretorioAtual[PATH_MAX];
	if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL) /* Obtem o nome do diretorio atual. */
	{
		printf("Erro ao obter diretorio atual para deletar o arquivo!\n");
		return;
	}
	
	//printf("NOME DO USUARIO QUE CHAMOU DELETE:\n%s\n", dados_conexao.nome_usuario);

	/* Concatena as informacoes necessarias em uma string, para deletar o arquivo. */
	char caminhoArquivo[PATH_MAX];
	strcat(caminhoArquivo,diretorioAtual);
	strcat(caminhoArquivo,"/");
	strcat(caminhoArquivo,PREFIXO_DIRETORIO);
	strcat(caminhoArquivo,dados_conexao.nome_usuario);
	strcat(caminhoArquivo,"/");
	strcat(caminhoArquivo,nomeArquivo);
	
	//printf("DIRETORIO PARA EXCLUIR ARQUIVO:\n%s\n", caminhoArquivo);
	
	/* Delete o arquivo presente na maquina local do usuario. */
	if (!remove(caminhoArquivo) == 0)
		printf("\nErro! Arquivo '%s' inexistente no diretorio local!\n", nomeArquivo);
	else
		printf("\nArquivo '%s' deletado do diretorio local com sucesso.\n", nomeArquivo);
	
    	memset(caminhoArquivo,0,sizeof(caminhoArquivo));
}

/* Lista os arquivos armazenados no repositorio remoto do servidor, associados ao usuario. */
void list_server(DadosConexao dados_conexao)
{
	limpaTela();
	
	/* Inicializa e copia os dados da comunicacao no pacote. */
	Pacote pacote;
	inicializaPacote(&pacote);
	pacote.codigoComunicacao = CODIGO_LISTSERVER; /* Identifica, no pacote, o codigo da operacao a ser realizada com o servidor. */
	strcpy(pacote.usuario,dados_conexao.nome_usuario);
	strcpy(pacote.nomePacote,NOME_REQUISICAO_LISTSERVER);
	
	/* Envia um pacote para requisitar ao servidor os dados do repositorio remoto. */
	if (write(dados_conexao.socket_id, (void*)&pacote, sizeof(pacote)) < 0)
	{
		printf("Erro! Nao foi possivel mandar o pacote para requisitar os dados dos arquivos do cliente no servidor!\n");
		return;
	}
	 
	/* Faz a leitura dos dados dos arquivos do repositorio remoto, que estao armazenados no conteudo do pacote. */
	if (read(dados_conexao.socket_id, &pacote, sizeof(pacote)) < 0) 
	{
		printf("Erro! Nao foi possivel realizar a leitura dos dados com o socket, para visualizar os dados dos arquivos do cliente no servidor!\n");
		return;
	}
	
	printf("ARQUIVOS SALVOS NO SERVIDOR, NO DIRETORIO DO USUARIO\n%s\n", dados_conexao.nome_usuario);
	printf("%s", pacote.conteudo); /* Exibe os dados dos arquivos do repositorio remoto. */
	printf("\n------------------------------------------------------------------------------\n");
}

void list_client(DadosConexao dados_conexao) /* Lista todos os arquivos do repositorio local syn_dir_<usuario>, junto com os MAC times. */
{
    limpaTela();
    
    char diretorioAtual[PATH_MAX];
    if (getcwd(diretorioAtual, sizeof(diretorioAtual)) == NULL) /* Obtem o nome do diretorio atual. */
    {
        printf("Erro ao obter diretorio atual.\n");
	return;
    }

    /* Concatena as informacoes de diretorio necessarias em uma string, para listar os arquivos. */   
    char caminhoSyncDir[PATH_MAX];
    strcat(caminhoSyncDir,diretorioAtual);
    strcat(caminhoSyncDir,"/");
    strcat(caminhoSyncDir,PREFIXO_DIRETORIO);
    strcat(caminhoSyncDir,dados_conexao.nome_usuario);
    listarArquivosDiretorio(caminhoSyncDir); /* Percorre o diretorio e lista todos os arquivos, junto com os MAC times. */
    memset(caminhoSyncDir,0,sizeof(caminhoSyncDir));
}
