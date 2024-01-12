#include "functions.hpp"
#include "defines.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <iostream>
#include "package_functions.hpp"
#include "package_file.hpp"

// Verifica se diretório existe, se não existir, cria
int create_dir_if_not_exists(const char *path)
{
    struct stat st = {};

    if (stat(path, &st) == -1)
    {
        if (mkdir(path, MASCARA_PERMISSAO))
        {
            return COULD_NOT_CREATE;
        }
    }

    return SUCCESS;
}

// Lista um diretório
std::optional<std::vector<File>> list_dir(const char *path)
{
    DIR *dp;
    struct dirent *entry;
    std::vector<File> files;

    dp = opendir(path);

    if (!dp)
    {
        return std::nullopt;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        // Ignora . e ..
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }

        std::string file_path = path;
        file_path.append("/");
        file_path.append(entry->d_name);

        // Obtém File do arquivo atual
        auto file_opt = file_from_path(file_path.c_str(), entry->d_name);

        if (!file_opt.has_value())
        {
            closedir(dp);
            return std::nullopt;
        }

        files.push_back(file_opt.value());
    }

    closedir(dp);

    return files;
}

// Lê File de determinado path, filename será o nome do arquivo em File
std::optional<File> file_from_path(const char *path, const char *filename)
{
    struct stat info;

    if (stat(path, &info) != 0)
    {
        return std::nullopt;
    }

    return File(info.st_size, info.st_mtime, info.st_atime, info.st_ctime, filename);
}
