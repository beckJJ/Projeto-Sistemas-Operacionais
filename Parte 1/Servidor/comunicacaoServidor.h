#ifndef _COMUNICACAOSERVIDOR_H_
#define _COMUNICACAOSERVIDOR_H_

#define PORT 4000

/* Constantes indicando o nome completo do diretorio principal do servidor. */
#define PREFIXO_DIRETORIO_SERVIDOR "sync_dir_SERVER"

/* Constante indicando a mascara de permissao de um diretorio a ser criado. */
#define MASCARA_PERMISSAO 0777

/* Constantes utilizadas para tamanhos de strings, usadas na interacao entre cliente e servidor. */
#define DIMENSAO_BUFFER 1024
#define DIMENSAO_GERAL 100
#define DIMENSAO_NOME_USUARIO 50

/* Constantes para nomear e identificar a utilidade de um pacote, que ira transitar entre o cliente e o servidor. */
#define NOME_REQUISICAO_LISTSERVER "Requisicao_list_server"
#define NOME_REQUISICAO_DOWNLOAD "Requisicao_download"
#define NOME_REQUISICAO_UPLOAD "Requisicao_upload"

/* Constantes para indicar os codigos das interacoes entre cliente e servidor, dependendo do comando escolhido pelo usuario. */
#define CODIGO_ERRO_DOWNLOAD -1
#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

/* Estrutura utilizada para agrupar os dados de um pacote, enviado durante a comunicacao cliente-servidor. */
typedef struct
{
	char conteudo[DIMENSAO_BUFFER];		/* Armazena o conteudo geral do pacote, podendo ser o conteudo de um arquivo ou informacoes sobre um diretorio remoto. */
	char nomePacote[DIMENSAO_GERAL];	/* Armazena o identificador do pacote, para deixar claro qual a operacao (comando escolhido pelo usuario) associada ele. */
	char nomeArquivo[DIMENSAO_GERAL];	/* Armazena o nome do arquivo sendo transferido, caso o pacote seja usado para transferencia de arquivos. */
	char usuario[DIMENSAO_NOME_USUARIO];	/* Armazena o usuario envolvido na operacao referente ao pacote, na comunicacao com o servidor. */
	
	int tamanho;				/* Indica a quantidade de bytes do pacote.*/
	int codigoComunicacao;			/* Valor inteiro para indicar a operacao (comando escolhido pelo usuario) associado ao pacote, usado tambem para identificar erros de comunicacao. */

} Pacote;

void download(int newsocket_id, Pacote *pacote);		/* Envia um pacote contendo um arquivo, que foi solicitado por um cliente. */
void listarArquivosDiretorio(char *diretorio, Pacote *pacote);	/* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente em um pacote. */
void list_server(int newsocket_id, Pacote *pacote);		/* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente em um pacote, e envia o pacote ao cliente solicitante. */
void upload(char *diretorioDestino, Pacote *pacote);		/* Armazena um arquivo enviado pelo cliente, no repositorio remoto associado a ele. */
void imprimeDadosPacote(Pacote pacote); 			/* Imprime, no terminal, os dados relacionados com um pacote que foi recebido pelo servidor ou enviado pelo servidor. */

#endif
