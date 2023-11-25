#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <optional>
#include <vector>
#include "package.hpp"

#define COULD_NOT_CREATE 1

// Verifica se diretório existe, se não existir, cria
int create_dir_if_not_exists(const char *path);

std::optional<std::vector<File>> list_dir(const char *path);

#endif
