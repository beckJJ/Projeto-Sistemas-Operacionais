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

#include "comunicacaoCliente.hpp"
#include "interfaceCliente.hpp"
#include "auxiliaresCliente.hpp"

/* Inicializa o pacote que ira conter os dados da interacao com o servidor. */
void inicializaPacote(Pacote *pacote) 
{
	memset(pacote->conteudo,0,sizeof(pacote->conteudo));
	memset(pacote->nomePacote,0,sizeof(pacote->nomePacote));
	memset(pacote->nomeArquivo,0,sizeof(pacote->nomeArquivo));
	memset(pacote->usuario,0,sizeof(pacote->usuario));
	pacote->tamanho = 0;
	pacote->codigoComunicacao = 0;
}

/* Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido. */
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

/* Cria ou carrega o diretorio sync_dir_<usuario> */
void analisa_diretorio(char *nome_usuario)
{
    char nome_diretorio[DIMENSAO_NOME_DIRETORIO];
    sprintf(nome_diretorio,PREFIXO_DIRETORIO);
    strcat(nome_diretorio,nome_usuario);

    struct stat st = {};

    if (stat(nome_diretorio, &st) == -1)
    {
        if (!(mkdir(nome_diretorio, MASCARA_PERMISSAO) == 0)) /* Criacao do diretorio do cliente, com controle dos casos em que nao e possivel cria-lo. */
        {
            printf("Erro ao criar o diretorio do cliente!\n");
            exit(EXIT_FAILURE);
	}
    }
}

/* Lista todos os arquivos de um determinado diretorio, junto com os MAC times. */
void listarArquivosDiretorio(char *diretorio) 
{
    DIR *dir;
    struct dirent *entrada;
    struct stat info;
    
    printf("ARQUIVOS DO DIRETORIO\n%s\n", diretorio);

    dir = opendir(diretorio);
    if (dir == NULL) /* Realiza a abertura do diretorio, para listar os arquivos. */
    {
            printf("Erro ao abrir o diretorio do cliente!\n");
            return;
    }
   
   /* Iteracao para percorrer todo o diretorio, imprimindo os dados MAC de cada arquivo. */
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
            return;
        }
    }

    closedir(dir);
}

/* Recebe um comando digitado pelo usuario, e obtem o nome do arquivo final referenciado no comando. */
void obtemNomeArquivoComando(char *comando, char *nomeArquivo)
{
	int indice = 0;
	while (comando[indice] != ' ')
		indice++;
	while (comando[indice] == ' ')
		indice++;
	int k = 0;
	for (int i = indice; comando[i] != '\0' && comando[i] != ' ' && comando[i] != '\t'; i++)
	{ 
		nomeArquivo[k] = comando[i];
		k++;
	}
	nomeArquivo[k] = '\0';
}

/* Recebe um comando digitado pelo usuario, e obtem o nome do diretorio referenciado no comando. */
void obtemDiretorioComando(char *comando, char *diretorio) 
{
	//printf("COMANDO RECEBIDO:\n%s\n", comando);
		
	int indice = 0;
	while (comando[indice] == ' ' || comando[indice] == '\t')
		indice++;
	while (comando[indice] != ' ')
		indice++;
	while (comando[indice] == ' ' || comando[indice] == '\t')
		indice++;
	int k = 0;
	for (int i = indice; i < strlen(comando); i++)
	{
		diretorio[k] = comando[i];
		k++;
	}
	diretorio[k] = '\0';
}

/* Recebe um diretorio escolhido pelo usuario, e obtem o nome do arquivo final referenciado no diretorio. */
void obtemNomeArquivoDiretorio(char *caminhoCompleto, char *nomeArquivo) 
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
