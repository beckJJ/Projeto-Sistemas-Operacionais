#ifndef _COMMON_COMUNICACAO_H_
#define _COMMON_COMUNICACAO_H_

/* Constantes para indicar os codigos das interacoes entre cliente e servidor, dependendo do comando escolhido pelo usuario. */
#define CODIGO_ERRO_DOWNLOAD -1
#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

/* Constantes para nomear e identificar a utilidade de um pacote, que ira transitar entre o cliente e o servidor. */
#define NOME_REQUISICAO_LISTSERVER "Requisicao_list_server"
#define NOME_REQUISICAO_DOWNLOAD "Requisicao_download"
#define NOME_REQUISICAO_UPLOAD "Requisicao_upload"

#define NOME_RESPOSTA_LISTSERVER "Resposta_list_server"
#define NOME_RESPOSTA_DOWNLOAD "Resposta_download"

#endif