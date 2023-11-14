#ifndef _AUXILIARESSERVIDOR_H_
#define _AUXILIARESSERVIDOR_H_

#define PORT 4000
#define PREFIXO_DIRETORIO_SERVIDOR "sync_dir_SERVER"
#define MASCARA_PERMISSAO 0777
#define DIMENSAO_BUFFER 1024
#define DIMENSAO_GERAL 100
#define DIMENSAO_NOME_USUARIO 50

#define ERRO_DOWNLOAD -1
#define CODIGO_UPLOAD 1
#define CODIGO_LISTSERVER 2
#define CODIGO_DOWNLOAD 3

#define NOME_RESPOSTA_LISTSERVER "Resposta_list_server"
#define NOME_RESPOSTA_DOWNLOAD "Resposta_download"

void analisa_diretorio_servidor(); /* Faz a criacao do diretorio sync_dir_SERVER, caso ele ainda nao exista. */
void criaNovoDiretorio(char *diretorioPai, char *nomeNovoDiretorio); /* Faz a criacao de um diretorio correspondente a um usuario, dentro do diretorio sync_dir_SERVER. */

#endif
