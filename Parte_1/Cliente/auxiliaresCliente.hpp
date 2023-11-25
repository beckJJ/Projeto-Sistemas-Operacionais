#ifndef _AUXILIARESCLIENTE_H_
#define _AUXILIARESCLIENTE_H_

#include <vector>
#include "../Common/package.hpp"

#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define TIME_FORMATTED_BUFFER 80
#define HEADER_FORMAT "%-30s %-7s %-20s %-20s %-20s\n"
#define FILE_FORMAT "%-30.30s %7ld %-20s %-20s %-20s\n"

std::vector<char *> splitComando(char *comando);
int verificaParametros(char *comando, int quantidade_parametros); /* Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido. */
void print_files(std::vector<File> files);

#endif
