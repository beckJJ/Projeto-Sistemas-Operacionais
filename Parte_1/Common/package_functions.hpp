#ifndef _PACKAGE_FUNCTIONS_H_
#define _PACKAGE_FUNCTIONS_H_

#include "package.hpp"
#include <optional>
#include <cstdio>

int read_package_from_socket(int socket, Package &package, std::vector<char> &fileContentBuffer);
int write_package_to_socket(int socket, Package &package, std::vector<char> &fileContentBuffer);
void print_package(FILE *fout, bool sending, Package &package, std::vector<char> &fileContentBuffer);

#endif
