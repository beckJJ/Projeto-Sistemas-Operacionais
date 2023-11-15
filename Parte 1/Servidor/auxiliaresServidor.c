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

#include "auxiliaresServidor.h"

/* Faz a criacao do diretorio sync_dir_SERVER, caso ele ainda nao exista. */
void analisa_diretorio_servidor()
{
    /* Criacao do novo diretorio remoto do usuario, com controle dos casos em que nao e possivel cria-lo. */
    struct stat st = {0};
    if (stat(PREFIXO_DIRETORIO_SERVIDOR, &st) == -1)
    {
        if (!(mkdir(PREFIXO_DIRETORIO_SERVIDOR, MASCARA_PERMISSAO) == 0)) 
        {
            printf("Erro ao criar o diretorio do servidor!\n");
            exit(EXIT_FAILURE);
	}
    }
}

/* Faz a criacao de um diretorio correspondente a um usuario, dentro do diretorio sync_dir_SERVER. */
void criaNovoDiretorio(char *diretorioPai, char *nomeNovoDiretorio) 
{
    char caminhoCompleto[PATH_MAX];
    
    /* Obtem o caminho completo no qual ocorrera a criacao do novo diretorio associado ao usuario. */
    snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", diretorioPai, nomeNovoDiretorio);

    /* Criacao do novo diretorio remoto do usuario, com controle dos casos em que nao e possivel cria-lo. */
    struct stat st = {0};
    if (stat(caminhoCompleto, &st) == -1) 
    {
        if (!(mkdir(caminhoCompleto, MASCARA_PERMISSAO) == 0))
        {
            printf("Erro ao criar o diretorio do usuario %s, dentro do diretorio %s!\n",nomeNovoDiretorio,PREFIXO_DIRETORIO_SERVIDOR);
            exit(EXIT_FAILURE);
        } 
    } 
}
