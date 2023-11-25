#ifndef _COMMON_DEFINES_H_
#define _COMMON_DEFINES_H_

#ifndef DEBUG_PACOTE
#define DEBUG_PACOTE false
#endif

#ifndef DEBUG_PACOTE_FILE_CONTENT
#define DEBUG_PACOTE_FILE_CONTENT false
#endif

#ifdef __linux__
#include <linux/limits.h>
#endif

/* Constante indicando a mascara de permissao de um diretorio a ser criado. */
#define MASCARA_PERMISSAO 0777

/* Constantes indicando o nome completo do diretorio principal do servidor. */
#define PREFIXO_DIRETORIO_SERVIDOR "sync_dir_SERVER"

/* Constantes indicando o prefixo do diretorio local, de cada usuario. */
#define PREFIXO_DIRETORIO "sync_dir_"

#define SUCCESS 0
#define GENERIC_ERROR 1

#endif
