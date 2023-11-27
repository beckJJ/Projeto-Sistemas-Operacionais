#ifndef _PACKAGE_FUNCTIONS_H_
#define _PACKAGE_FUNCTIONS_H_

#include "package.hpp"
#include <optional>
#include <cstdio>

// Lê um pacote do socket, aplica automaticamente a conversão necessária e debuga pacote caso DEBUG_PACOTE
//   package terá o pacote lido e fileContentBuffer o conteúdo do arquivo do pacote atual
int read_package_from_socket(int socket, Package &package, std::vector<char> &fileContentBuffer);

// Escreve um pacote do socket, aplica automaticamente a conversão necessária e debuga pacote caso DEBUG_PACOTE
//   package será alterado duas vezes: inicialmente com htobe() e depois com betoh()
int write_package_to_socket(int socket, Package &package, std::vector<char> &fileContentBuffer);

// Exibe informações sobre um pacote
void print_package(FILE *fout, bool sending, Package &package, std::vector<char> &fileContentBuffer);

#endif
