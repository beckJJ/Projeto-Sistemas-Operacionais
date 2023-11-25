#include "package_commands.hpp"
#include "package_functions.hpp"
#include <sys/stat.h>
#include <algorithm>
#include <iostream>

int send_file_list(int socket, std::vector<File> files)
{
    std::vector<char> fileContentBuffer;

    if (files.size() == 0)
    {
        auto package = Package(PackageFileList(0, 0, File()));

        return write_package_to_socket(socket, package, fileContentBuffer);
    }

    for (size_t i = 0; i < files.size(); i++)
    {
        auto package = Package(PackageFileList(files.size(), i + 1, files[i]));

        if (write_package_to_socket(socket, package, fileContentBuffer))
        {
            return -1;
        }
    }

    return 0;
}

int send_file(int socket, const char *path)
{
    Package package;
    FILE *fp = fopen(path, "rb");
    struct stat st = {};

    if (!fp)
    {
        return -1;
    }

    if (stat(path, &st) == -1)
    {
        return -2;
    }

    auto fsize = st.st_size;
    auto to_read = fsize;

    std::vector<char> buffer(std::min((long)MAX_DATA_SIZE, to_read));

    for (auto seqn = 1; to_read > 0; seqn++)
    {
        size_t to_read_now = std::min((long)MAX_DATA_SIZE, to_read);
        auto readed = fread(buffer.data(), sizeof(char), to_read_now, fp);

        if (to_read_now != readed)
        {
            goto fail;
        }

        to_read -= readed;

        package = Package(PackageFileContent(PackageFileContentBase(fsize, seqn, readed)));

        if (write_package_to_socket(socket, package, buffer))
        {
            goto fail;
        }
    }

    fclose(fp);

    return 0;

fail:
    fclose(fp);

    return -1;
}

std::optional<std::vector<File>> read_file_list(int socket)
{
    Package package;
    std::vector<File> files;
    size_t total_files = 0;
    bool first_package = true;
    std::vector<char> fileContentBuffer;

    while (true)
    {
        if (read_package_from_socket(socket, package, fileContentBuffer))
        {
            return std::nullopt;
        }

        if (package.package_type != FILE_LIST)
        {
            return std::nullopt;
        }

        // Se for o primeiro pacote obtemos a informação sobre quantos arquivos há
        if (first_package)
        {
            total_files = package.package_specific.fileList.count;
            first_package = false;

            if (total_files == 0)
            {
                return std::vector<File>(0);
            }
        }

        files.push_back(package.package_specific.fileList.file);

        if (total_files == files.size())
        {
            break;
        }
    }

    return files;
}

int read_file_and_save(int socket, const char *path)
{
    FILE *fp = fopen(path, "wb");
    bool first_package = true;
    int32_t total_write = 0;
    int32_t fsize = 0;
    Package package;
    std::vector<char> fileContentBuffer;

    if (!fp)
    {
        return -1;
    }

    while (true)
    {
        if (read_package_from_socket(socket, package, fileContentBuffer))
        {
            goto fail;
        }

        if (package.package_type != FILE_CONTENT)
        {
            goto fail;
        }

        // Se for o primeiro pacote obtemos a informação sobre tamanho
        if (first_package)
        {
            fsize = package.package_specific.fileContent.size;
            first_package = false;
        }

        auto ret = fwrite(
            fileContentBuffer.data(),
            sizeof(char),
            package.package_specific.fileContent.data_length,
            fp);

        if (ret != package.package_specific.fileContent.data_length)
        {
            goto fail;
        }

        total_write += ret;

        if (total_write == fsize)
        {
            break;
        }
    }

    fclose(fp);

    return 0;

fail:
    fclose(fp);
    remove(path);

    return -1;
}
