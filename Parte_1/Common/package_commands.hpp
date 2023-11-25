#ifndef _PACKAGE_COMMANDS_H_
#define _PACKAGE_COMMANDS_H_

#include "defines.hpp"
#include "package.hpp"
#include <optional>

// Envia uma sequência de pacotes PackageFileList contendo as informações de files
int send_file_list(int socket, std::vector<File> files);

// Envia uma sequência de pacotes PackageFileContent contendo o conteúdo em path
int send_file(int socket, const char *path);

// Lê uma sequência de pacotes PackageFileList e retorna um vetor contendo os pacotes
std::optional<std::vector<File>> read_file_list(int socket);

// Lê uma sequência de pacotes PackageFileContent e armazena o conteúdo obtido em path
int read_file_and_save(int socket, const char *path);

#endif
