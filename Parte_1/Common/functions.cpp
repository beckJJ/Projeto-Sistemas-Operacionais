#include "functions.hpp"
#include "defines.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <iostream>

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

std::optional<std::vector<File>> list_dir(const char *path)
{
    DIR *dp;
    struct dirent *entry;
    struct stat info;
    std::vector<File> files;

    dp = opendir(path);

    if (!dp)
    {
        return std::nullopt;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }

        std::string file_path = path;
        file_path.append("/");
        file_path.append(entry->d_name);

        if (stat(file_path.c_str(), &info) == 0)
        {
            files.push_back(File(info.st_size, info.st_mtime, info.st_atime, info.st_ctime, entry->d_name));
        }
        else
        {
            closedir(dp);
            return std::nullopt;
        }
    }

    closedir(dp);

    return files;
}
