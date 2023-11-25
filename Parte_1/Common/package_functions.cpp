#include "package_functions.hpp"
#include <unistd.h>
#include <iostream>
#include <string.h>

std::optional<std::vector<char>> read_n_bytes_from_socket(int socket, ssize_t size);
std::optional<PackageType> read_package_type_from_socket(int socket);
std::optional<ssize_t> sizeof_base_package(PackageType package_type);
int write_n_bytes_from_socket(int socket, char *buffer, ssize_t size);

std::optional<std::vector<char>> read_n_bytes_from_socket(int socket, ssize_t size)
{
    std::vector<char> buffer = std::vector<char>(size);
    ssize_t readed_bytes_count = 0;
    ssize_t readed_bytes = 0;

    while (size)
    {
        readed_bytes = read(socket, &buffer.data()[readed_bytes_count], size);

        if (readed_bytes == -1)
        {
            std::cout << "Socket read returned -1." << std::endl;
            return std::nullopt;
        }
        else if (readed_bytes == 0)
        {
            std::cout << "Socket read returned 0, needed to read " << size << " bytes." << std::endl;
            return std::nullopt;
        }

        readed_bytes_count += readed_bytes;
        size -= readed_bytes;
    }

    return buffer;
}

std::optional<PackageType> read_package_type_from_socket(int socket)
{
    std::optional<std::vector<char>> buffer = read_n_bytes_from_socket(socket, ALIGN_VALUE);

    if (!buffer.has_value())
    {
        return std::nullopt;
    }

    PackageType package_type = *(PackageType *)buffer.value().data();

    return package_type;
}

std::optional<ssize_t> sizeof_base_package(PackageType package_type)
{
    switch (package_type)
    {
    case INITAL_USER_INDENTIFICATION:
        return sizeof(PackageUserIndentification);
    case USER_INDENTIFICATION_RESPONSE:
        return sizeof(PackageUserIndentificationResponse);
    case CHANGE_EVENT:
        return sizeof(PackageChangeEvent);
    case REQUEST_FILE:
        return sizeof(PackageRequestFile);
    case FILE_CONTENT:
        // Não sabemos o tamanho de data no momento
        return sizeof(PackageFileContentBase);
    case REQUEST_FILE_LIST:
        return sizeof(PackageRequestFileList);
    case FILE_LIST:
        return sizeof(PackageFileList);
    case UPLOAD_FILE:
        return sizeof(PackageUploadFile);
    case FILE_NOT_FOUND:
        return sizeof(PackageFileNotFound);
    default:
        printf("[sizeof_base_package] Unknown package type: 0x%02x\n", (uint8_t)package_type);
        return std::nullopt;
    }
}

int read_package_from_socket(int socket, Package &package, std::vector<char> &fileContentBuffer)
{
    std::optional<PackageType> package_type = read_package_type_from_socket(socket);

    if (!package_type.has_value())
    {
        return 1;
    }

    std::optional<ssize_t> size_to_read_opt = sizeof_base_package(package_type.value());

    if (!size_to_read_opt.has_value())
    {
        return 1;
    }

    std::optional<std::vector<char>> buffer = read_n_bytes_from_socket(socket, size_to_read_opt.value());

    if (!buffer.has_value())
    {
        return 1;
    }

    char *buffer_data = buffer.value().data();

    switch (package_type.value())
    {
    case INITAL_USER_INDENTIFICATION:
        package = Package(PackageUserIndentification(buffer_data));
        break;
    case USER_INDENTIFICATION_RESPONSE:
        package = Package(PackageUserIndentificationResponse(
            *(InitialUserIndentificationResponseStatus *)buffer_data,
            buffer_data[ALIGN_VALUE]));
        break;
    case CHANGE_EVENT:
        package = Package(PackageChangeEvent(
            *(ChangeEvents *)buffer_data,
            &buffer_data[ALIGN_VALUE]));
        break;
    case REQUEST_FILE:
        package = Package(PackageRequestFile(buffer_data));
        break;
    case FILE_CONTENT:
    {
        package = Package(PackageFileContentBase(
            be64toh(*(int64_t *)buffer_data),
            be16toh(*(uint16_t *)&buffer_data[ALIGN_VALUE]),
            be16toh(*(uint16_t *)&buffer_data[2 * ALIGN_VALUE])));

        // Lê conteúdo do arquivo
        auto fileContent = read_n_bytes_from_socket(socket, package.package_specific.fileContent.data_length);

        if (!fileContent.has_value())
        {
            return 1;
        }

        fileContentBuffer = fileContent.value();
        break;
    }
    case REQUEST_FILE_LIST:
        package = Package(PackageRequestFileList());
        break;
    case FILE_LIST:
        package = Package(PackageFileList(
            be16toh(*(uint16_t *)buffer_data),
            be16toh(*(uint16_t *)&(buffer_data[ALIGN_VALUE])),
            File(
                be64toh(*(int64_t *)&(buffer_data[2 * ALIGN_VALUE])),
                be64toh(*(int64_t *)&(buffer_data[3 * ALIGN_VALUE])),
                be64toh(*(int64_t *)&(buffer_data[4 * ALIGN_VALUE])),
                be64toh(*(int64_t *)&(buffer_data[5 * ALIGN_VALUE])),
                &buffer_data[6 * ALIGN_VALUE])));
        break;
    case UPLOAD_FILE:
        package = Package(PackageUploadFile(
            File(
                be64toh(*(int64_t *)buffer_data),
                be64toh(*(int64_t *)&(buffer_data[ALIGN_VALUE])),
                be64toh(*(int64_t *)&(buffer_data[2 * ALIGN_VALUE])),
                be64toh(*(int64_t *)&(buffer_data[3 * ALIGN_VALUE])),
                &buffer_data[4 * ALIGN_VALUE])));
        break;
    case FILE_NOT_FOUND:
        package = Package(PackageFileNotFound());
        break;
    default:
        return 1;
    }

#if DEBUG_PACOTE
    print_package(stderr, false, package, fileContentBuffer);
#endif

    return 0;
}

int write_n_bytes_from_socket(int socket, char *buffer, ssize_t size)
{
    ssize_t writed_bytes_count = 0;
    ssize_t writed_bytes = 0;

    while (size)
    {
        writed_bytes = write(socket, &buffer[writed_bytes_count], size);

        if (writed_bytes == -1)
        {
            std::cout << "Socket write returned -1." << std::endl;
            return -1;
        }
        else if (writed_bytes == 0)
        {
            std::cout << "Socket write returned 0, needed to write " << size << " bytes." << std::endl;
            return -1;
        }

        writed_bytes_count += writed_bytes;
        size -= writed_bytes;
    }

    return 0;
}

int write_package_to_socket(int socket, Package &package, std::vector<char> &fileContentBuffer)
{
    // Escreve apenas os bytes necessários
    auto size_base_package = sizeof_base_package(package.package_type);

    if (!size_base_package.has_value())
    {
        return -1;
    }

    ssize_t size_to_write = ALIGN_VALUE + size_base_package.value();

#if DEBUG_PACOTE
    print_package(stderr, true, package, fileContentBuffer);
#endif

    // Converte Package para representação be
    package.htobe();

    // Tenta enviar o pacote
    int failed = write_n_bytes_from_socket(socket, (char *)&package, size_to_write);

    // Retorna a representação local
    package.betoh();

    if (failed)
    {
        return -1;
    }

    // Apenas FILE_CONTENT tem conteúdo a mais
    if (package.package_type != FILE_CONTENT)
    {
        return 0;
    }

    // Envia conteúdo do arquivo
    return write_n_bytes_from_socket(socket, fileContentBuffer.data(), package.package_specific.fileContent.data_length);
}

void print_package(FILE *fout, bool sending, Package &package, std::vector<char> &fileContentBuffer)
{
    if (sending)
    {
        fprintf(fout, "WRITE ");
    }
    else
    {
        fprintf(fout, "READ  ");
    }

    switch (package.package_type)
    {
    case INITAL_USER_INDENTIFICATION:
        fprintf(fout, "Package(INITAL_USER_INDENTIFICATION, %s", package.package_specific.userIdentification.user_name);
        break;
    case USER_INDENTIFICATION_RESPONSE:
        fprintf(fout, "Package(USER_INDENTIFICATION_RESPONSE, ");

        switch (package.package_specific.userIdentificationResponse.status)
        {
        case ACCEPTED:
            fprintf(fout, "ACCEPTED");
            break;
        case REJECTED:
            fprintf(fout, "ACCEPTED");
            break;
        default:
            fprintf(fout, "UNKNOWN");
            break;
        }

        fprintf(fout, ", %d", package.package_specific.userIdentificationResponse.deviceID);
        break;
    case CHANGE_EVENT:
        fprintf(fout, "Package(CHANGE_EVENT, ");

        switch (package.package_specific.ChangeEvent.event)
        {
        case FILE_DELETED:
            fprintf(fout, "FILE_DELETED, ");
            break;
        case FILE_CREATED:
            fprintf(fout, "FILE_CREATED, ");
            break;
        case FILE_MODIFIED:
            fprintf(fout, "FILE_MODIFIED, ");
            break;
        default:
            fprintf(fout, "UNKNOWN, ");
            break;
        }

        fprintf(fout, "%s", package.package_specific.ChangeEvent.filename);
        break;
    case REQUEST_FILE:
        fprintf(fout, "Package(REQUEST_FILE, %s", package.package_specific.requestFile.filename);
        break;
    case FILE_CONTENT:
        fprintf(
            fout,
            "Package(FILE_CONTENT, %ld, %d, %d, data[%ld]=\"",
            package.package_specific.fileContent.size,
            package.package_specific.fileContent.seqn,
            package.package_specific.fileContent.data_length,
            fileContentBuffer.size());

#if DEBUG_PACOTE_FILE_CONTENT
        for (auto i = 0; i < package.package_specific.fileContent.data_length; i++)
        {
            fprintf(fout, "%02x ", (uint8_t)fileContentBuffer[i]);
        }
#endif

        fprintf(fout, "\"");

        break;
    case REQUEST_FILE_LIST:
        fprintf(fout, "Package(REQUEST_FILE_LIST");
        break;
    case FILE_LIST:
        fprintf(
            fout,
            "Package(FILE_LIST, %d, %d, File(%ld, %ld, %ld, %ld, %s)",
            package.package_specific.fileList.count,
            package.package_specific.fileList.seqn,
            package.package_specific.fileList.file.size,
            package.package_specific.fileList.file.mtime,
            package.package_specific.fileList.file.atime,
            package.package_specific.fileList.file.ctime,
            package.package_specific.fileList.file.name);
        break;
    case UPLOAD_FILE:
        fprintf(
            fout,
            "Package(UPLOAD_FILE, File(%ld, %ld, %ld, %ld, %s)",
            package.package_specific.uploadFile.size,
            package.package_specific.uploadFile.mtime,
            package.package_specific.uploadFile.atime,
            package.package_specific.uploadFile.ctime,
            package.package_specific.uploadFile.name);
        break;
    case FILE_NOT_FOUND:
        fprintf(fout, "Package(FILE_NOT_FOUND");
        break;
    default:
        fprintf(fout, "Package(UNKOWN");
        break;
    }

    fprintf(fout, ")\n");
}
