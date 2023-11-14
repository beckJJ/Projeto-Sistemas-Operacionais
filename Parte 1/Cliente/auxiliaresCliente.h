#ifndef _AUXILIARESCLIENTE_H_
#define _AUXILIARESCLIENTE_H_

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
#define ERRO_DOWNLOAD -1
#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

void inicializaPacote(Pacote *pacote); /* Inicializa o pacote que ira conter os dados da interacao com o servidor. */
int verificaParametros(char *comando, int quantidade_parametros); /* Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido. */
void analisa_diretorio(char *nome_usuario); /* Cria ou carrega o diretorio sync_dir_<usuario> */
void listarArquivosDiretorio(char *diretorio); /* Lista todos os arquivos de um determinado diretorio, junto com os MAC times. */
void obtemNomeArquivoComando(char *comando, char *nomeArquivo); /* Recebe um comando digitado pelo usuario, e obtem o nome do arquivo final referenciado no comando. */
void obtemDiretorioComando(char *comando, char *diretorio); /* Recebe um comando digitado pelo usuario, e obtem o nome do diretorio referenciado no comando. */
void obtemNomeArquivoDiretorio(char *caminhoCompleto, char *nomeArquivo); /* Recebe um diretorio escolhido pelo usuario, e obtem o nome do arquivo final referenciado no diretorio. */

#endif
