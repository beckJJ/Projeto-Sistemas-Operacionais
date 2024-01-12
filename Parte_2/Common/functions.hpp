#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

// Funções gerais comuns para tanto servidor e cliente

#include <optional>
#include <vector>
#include "package.hpp"

#define COULD_NOT_CREATE 1

// Verifica se diretório existe, se não existir, cria
int create_dir_if_not_exists(const char *path);

// Lista um diretório
std::optional<std::vector<File>> list_dir(const char *path);

// Lê File de determinado path, filename será o nome do arquivo em File
std::optional<File> file_from_path(const char *path, const char *filename);

#endif
