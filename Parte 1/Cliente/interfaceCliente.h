#ifndef _INTERFACECLIENTE_H_
#define _INTERFACECLIENTE_H_

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

void limpaTela();
void menu_help();
void menu_principal(DadosConexao dados_conexao);
int executa_comando(DadosConexao dados_conexao);

#endif

