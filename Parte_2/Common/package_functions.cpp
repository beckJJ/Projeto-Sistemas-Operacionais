#include "package_functions.hpp"
#include <unistd.h>
#include <iostream>
#include <string.h>

#if DEBUG_PACOTE
// Lock usado por print_package para exibir completamente um pacote
extern pthread_mutex_t print_package_lock;
#endif

// Lê n bytes de um socket, caso sucesso o vetor terá o conteúdo lido
std::optional<std::vector<char>>
read_n_bytes_from_socket(int socket, ssize_t size);

// Lê o tipo do pacote de um socket
std::optional<PackageType> read_package_type_from_socket(int socket);

// Tamanho de um pacote dependendo de seu package_type (não contabiliza o PackageType de Package)
std::optional<ssize_t> sizeof_base_package(PackageType package_type);

// Escreve n bytes em um socket
int write_n_bytes_from_socket(int socket, char *buffer, ssize_t size);

// Lê n bytes de um socket, caso sucesso o vetor terá o conteúdo lido
std::optional<std::vector<char>> read_n_bytes_from_socket(int socket, ssize_t size)
{
    std::vector<char> buffer = std::vector<char>(size);
    ssize_t readed_bytes_count = 0;
    ssize_t readed_bytes = 0;

    // Lê até ter lido size bytes
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

// Lê o tipo do pacote de um socket
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

// Tamanho de um pacote dependendo de seu package_type (não contabiliza o PackageType de Package)
std::optional<ssize_t> sizeof_base_package(PackageType package_type)
{
    switch (package_type)
    {
    case INITIAL_USER_IDENTIFICATION:
        return sizeof(PackageUserIdentification);
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
        return sizeof(PackageReplicaManagerIdentification);
    case USER_IDENTIFICATION_RESPONSE:
        return sizeof(PackageUserIdentificationResponse);
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
        return sizeof(PackageReplicaManagerIdentificationResponse);
    case CHANGE_EVENT:
        return sizeof(PackageChangeEvent);
    case REQUEST_FILE:
        return sizeof(PackageRequestFile);
    case FILE_CONTENT:
        return sizeof(PackageFileContent);
    case REQUEST_FILE_LIST:
        return sizeof(PackageRequestFileList);
    case FILE_LIST:
        return sizeof(PackageFileList);
    case UPLOAD_FILE:
        return sizeof(PackageUploadFile);
    case FILE_NOT_FOUND:
        return sizeof(PackageFileNotFound);
    case REPLICA_MANAGER_PING:
        return sizeof(PackageReplicaManagerPing);
    case REPLICA_MANAGER_PING_RESPONSE:
        return sizeof(PackageReplicaManagerPingResponse);
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        return sizeof(PackageReplicaManagerTransferIdentification);
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
        return sizeof(PackageReplicaManagerTransferIdentificationResponse);
    default:
        printf("[sizeof_base_package] Unknown package type: 0x%02x\n", (uint8_t)package_type);
        return std::nullopt;
    }
}

// Lê um pacote do socket, aplica automaticamente a conversão de bexxtoh e debuga pacote caso DEBUG_PACOTE
//   package terá o pacote lido e fileContentBuffer o conteúdo do arquivo do pacote atual
int read_package_from_socket(int socket, Package &package, std::vector<char> &fileContentBuffer)
{
    // Obtém PackageType do pacote atual
    std::optional<PackageType> package_type = read_package_type_from_socket(socket);

    if (!package_type.has_value())
    {
        return 1;
    }

    // Obtém tamanho do pacote que deve ser lido
    std::optional<ssize_t> size_to_read_opt = sizeof_base_package(package_type.value());

    if (!size_to_read_opt.has_value())
    {
        return 1;
    }

    // Lê o tamanho necessário
    std::optional<std::vector<char>> buffer = read_n_bytes_from_socket(socket, size_to_read_opt.value());

    if (!buffer.has_value())
    {
        return 1;
    }

    char *buffer_data = buffer.value().data();

    // Cria Package dependendo tipo, aplicando bexxtoh conforme necessário
    switch (package_type.value())
    {
    case INITIAL_USER_IDENTIFICATION:
        package = Package(PackageUserIdentification(
            *(uint8_t *)buffer_data,
            &buffer_data[ALIGN_VALUE]));
        break;
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
        package = Package(PackageReplicaManagerIdentification(
            *(uint8_t*)buffer_data));
        break;
    case USER_IDENTIFICATION_RESPONSE:
        package = Package(PackageUserIdentificationResponse(
            *(InitialUserIdentificationResponseStatus *)buffer_data,
            *(uint8_t *)&buffer_data[ALIGN_VALUE]));
        break;
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
        package = Package(PackageReplicaManagerIdentificationResponse(
            *(InitialReplicaManagerIdentificationResponseStatus*)buffer_data,
            *(uint8_t*)&buffer_data[ALIGN_VALUE]));
        break;
    case CHANGE_EVENT:
        package = Package(PackageChangeEvent(
            *(ChangeEvents *)buffer_data,
            *(uint8_t *)&buffer_data[ALIGN_VALUE],
            &buffer_data[2 * ALIGN_VALUE],
            // NAME_MAX == 255 e alignas(8), logo haverá 1 byte de padding
            &buffer_data[2 * ALIGN_VALUE + NAME_MAX + 1]));
        break;
    case REQUEST_FILE:
        package = Package(PackageRequestFile(buffer_data));
        break;
    case FILE_CONTENT:
    {
        package = Package(PackageFileContent(
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
    case REPLICA_MANAGER_PING:
        package = Package(PackageReplicaManagerPing());
        break;
    case REPLICA_MANAGER_PING_RESPONSE:
        package = Package(PackageReplicaManagerPingResponse());
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        package = Package(PackageReplicaManagerTransferIdentification(
            *(uint8_t*)buffer_data));
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
        package = Package(PackageReplicaManagerTransferIdentificationResponse(
            *(ReplicaManagerTransferIdentificationResponseStatus*)buffer_data,
            *(uint8_t*)&buffer_data[ALIGN_VALUE]));
        break;
    default:
        return 1;
    }

    // Debuga caso DEBUG_PACOTE habilitado
#if DEBUG_PACOTE
    print_package(stderr, false, package, fileContentBuffer);
#endif

    return 0;
}

// Escreve n bytes em um socket
int write_n_bytes_from_socket(int socket, char *buffer, ssize_t size)
{
    ssize_t writed_bytes_count = 0;
    ssize_t writed_bytes = 0;

    // Escreve até ter escrito size bytes
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
    // Tamanho do pacote específico sem PackageType
    auto size_base_package = sizeof_base_package(package.package_type);

    if (!size_base_package.has_value())
    {
        return -1;
    }

    // Tamanho do pacote a ser escrito
    ssize_t size_to_write = ALIGN_VALUE + size_base_package.value();

    // Converte Package para representação be
    package.htobe();

    // Envia o pacote
    int failed = write_n_bytes_from_socket(socket, (char *)&package, size_to_write);

    // Retorna a representação local
    package.betoh();

    if (failed)
    {
        return -1;
    }

    // Debuga caso DEBUG_PACOTE habilitado
#if DEBUG_PACOTE
    print_package(stderr, true, package, fileContentBuffer);
#endif

    // Apenas FILE_CONTENT tem conteúdo a mais
    if (package.package_type != FILE_CONTENT)
    {
        return 0;
    }

    // Envia conteúdo do arquivo
    return write_n_bytes_from_socket(socket, fileContentBuffer.data(), package.package_specific.fileContent.data_length);
}

// Exibe informações sobre um pacote
void print_package(FILE *fout, bool sending, Package &package, std::vector<char> &fileContentBuffer)
{
#if DEBUG_PACOTE
    pthread_mutex_lock(&print_package_lock);
#endif

#if DEBUG_PACOTE_TID
#ifndef __APPLE__
    pid_t tid = gettid();
#else
    // macOS nao tem gettid()
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    pid_t tid = (pid_t)tid;
#endif

    fprintf(fout, "[tid: %d] ", tid);
#endif

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
    case INITIAL_USER_IDENTIFICATION:
        fprintf(fout,
                "Package(INITIAL_USER_IDENTIFICATION, 0x%02x, %s",
                (uint8_t)package.package_specific.userIdentification.deviceID,
                package.package_specific.userIdentification.user_name);
        break;
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
        fprintf(fout,
                "Package(INITIAL_REPLICA_MANAGER_IDENTIFICATION, 0x%02x",
                (uint8_t)package.package_specific.replicaManagerIdentification.deviceID);
        break;
    case USER_IDENTIFICATION_RESPONSE:
        fprintf(fout, "Package(USER_IDENTIFICATION_RESPONSE, ");

        switch (package.package_specific.userIdentificationResponse.status)
        {
        case ACCEPTED:
            fprintf(fout, "ACCEPTED");
            break;
        case REJECTED:
            fprintf(fout, "REJECTED");
            break;
        default:
            fprintf(fout, "UNKNOWN");
            break;
        }

        fprintf(fout, ", %d", package.package_specific.userIdentificationResponse.deviceID);
        break;
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
        fprintf(fout, "Package(REPLICA_MANAGER_IDENTIFICATION_RESPONSE, ");
        switch (package.package_specific.replicaManagerIdentificationResponse.status) {
        case ACCEPTED_RM:
            fprintf(fout, "ACCEPTED_RM");
            break;
        case REJECTED_RM:
            fprintf(fout, "REJECTED_RM");
            break;
        default:
            fprintf(fout, "UNKNOWN_RM");
            break;
        }
        break;
    case CHANGE_EVENT:
        fprintf(fout, "Package(CHANGE_EVENT, 0x%02x, ", (uint8_t)package.package_specific.changeEvent.deviceID);

        switch (package.package_specific.changeEvent.event)
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
        case FILE_RENAME:
            fprintf(fout, "FILE_RENAME, ");
            break;
        default:
            fprintf(fout, "UNKNOWN, ");
            break;
        }

        fprintf(fout, "%s, ", package.package_specific.changeEvent.filename1);
        fprintf(fout, "%s", package.package_specific.changeEvent.filename2);
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

        // Exibe conteúdo em hex caso desejado
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
    case REPLICA_MANAGER_PING:
        fprintf(fout, "Package(REPLICA_MANAGER_PING");
        break;
    case REPLICA_MANAGER_PING_RESPONSE:
        fprintf(fout, "Package(REPLICA_MANAGER_PING_RESPONSE");
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        fprintf(fout, "Package(REPLICA_MANAGER_TRANSFER_IDENTIFICATION");
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
        fprintf(fout, "Package(REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE");
        break;
    default:
        fprintf(fout, "Package(UNKOWN[0x%02x]", (uint8_t)package.package_type);
        break;
    }

    fprintf(fout, ")\n");

#if DEBUG_PACOTE
    pthread_mutex_unlock(&print_package_lock);
#endif
}
