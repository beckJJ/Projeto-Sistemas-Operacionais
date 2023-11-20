#ifndef _COMMON_DEFINES_H_
#define _COMMON_DEFINES_H_

#ifdef __linux__
#include <linux/limits.h>
#endif

/* Constantes utilizadas para tamanhos de strings, usadas na interacao entre cliente e servidor. */
#define DIMENSAO_GERAL 100
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50
#define DIMENSAO_NOME_DIRETORIO 200
#define DIMENSAO_BUFFER 1024

/* Constante indicando a mascara de permissao de um diretorio a ser criado. */
#define MASCARA_PERMISSAO 0777

/* Constantes indicando o nome completo do diretorio principal do servidor. */
#define PREFIXO_DIRETORIO_SERVIDOR "sync_dir_SERVER"

/* Constantes indicando o prefixo do diretorio local, de cada usuario. */
#define PREFIXO_DIRETORIO "sync_dir_"

#define SUCCESS         0
#define GENERIC_ERROR   1

#endif
