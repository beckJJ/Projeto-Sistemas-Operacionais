#ifndef _AUXILIARESSERVIDOR_H_
#define _AUXILIARESSERVIDOR_H_

#include "../Common/structs.hpp"
#include "../Common/defines.hpp"

void analisa_diretorio_servidor(); /* Faz a criacao do diretorio sync_dir_SERVER, caso ele ainda nao exista. */
int criaNovoDiretorio(const char *diretorioPai, char *nomeNovoDiretorio); /* Faz a criacao de um diretorio correspondente a um usuario, dentro do diretorio sync_dir_SERVER. */

#endif
