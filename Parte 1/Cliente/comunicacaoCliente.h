#ifndef _COMUNICACAOCLIENTE_H_
#define _COMUNICACAOCLIENTE_H_

/* Constantes utilizadas para tamanhos de strings, usadas na interacao entre cliente e servidor. */
#define DIMENSAO_GERAL 100
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50
#define DIMENSAO_NOME_DIRETORIO 200
#define DIMENSAO_BUFFER 1024

/* Constante indicando a mascara de permissao de um diretorio a ser criado. */
#define MASCARA_PERMISSAO 0777

/* Constantes indicando o prefixo do diretorio local, de cada usuario. */
#define PREFIXO_DIRETORIO "sync_dir_"

/* Constantes para indicar a sintaxe de cada comando. */
#define COMANDO_MYCLIENT "myClient"
#define COMANDO_UPLOAD "upload"
#define COMANDO_DOWNLOAD "download"
#define COMANDO_DELETE "delete"
#define COMANDO_LISTSERVER "list_server"
#define COMANDO_LISTCLIENT "list_client"
#define COMANDO_EXIT "exit"
#define COMANDO_HELP "help"

/* Constantes para indicar a quantidade de parametros de cada comando. */
#define QUANTIDADE_PARAMETROS_MYCLIENT 3
#define QUANTIDADE_PARAMETROS_UPLOAD 1
#define QUANTIDADE_PARAMETROS_DOWNLOAD 1
#define QUANTIDADE_PARAMETROS_DELETE 1

/* Constantes para nomear e identificar a utilidade de um pacote, que ira transitar entre o cliente e o servidor. */
#define NOME_REQUISICAO_LISTSERVER "Requisicao_list_server"
#define NOME_REQUISICAO_DOWNLOAD "Requisicao_download"
#define NOME_REQUISICAO_UPLOAD "Requisicao_upload"

/* Constantes para indicar os codigos das interacoes entre cliente e servidor, dependendo do comando escolhido pelo usuario. */
#define CODIGO_ERRO_DOWNLOAD -1
#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

/* Estrutura utilizada para conter todos os dados da conexao. */
typedef struct
{
	char nome_usuario[DIMENSAO_NOME_USUARIO]; 	/* Armazena o nome do usuario que estabeleceu conexao com o servidor. */
	char endereco_ip[DIMENSAO_GERAL]; 		/* Armazena o endereco IP do servidor. */
	char numero_porta[DIMENSAO_GERAL]; 		/* Armazena o numero da porta, usado para estabelecer conexao cliente-servidor. */
	
	char comando[DIMENSAO_COMANDO]; 		/* Armazena o comando da vez, escolhido pelo usuario. */
	
	int socket_id; 					/* Armazena o valor inteiro que identifica o socket sendo utilizado para comunicacao. */
	
} DadosConexao;

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

int conecta_servidor(DadosConexao dados_conexao); /* Inicia a conexao com o servidor, via socket. */
void upload(DadosConexao dados_conexao); /* Realiza o envio de um arquivo para o diretorio do usuario, presente no servidor. */
void download (DadosConexao dados_conexao); /* Recebe um arquivo do servidor, e copia para o diretorio de onde a aplicacao cliente foi chamada. */
void delete(DadosConexao dados_conexao); /* Deleta um arquivo presente na maquina local do usuario, no diretorio sync_dir_<usuario> */
void list_server(DadosConexao dados_conexao); /* Lista os arquivos armazenados no repositorio remoto do servidor, associados ao usuario. */
void list_client(DadosConexao dados_conexao); /* Lista todos os arquivos do repositorio local syn_dir_<usuario>, junto com os MAC times. */

#endif
