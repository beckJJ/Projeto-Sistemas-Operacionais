#include "package_file.hpp"
#include "package_functions.hpp"
#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include "functions.hpp"
#include "connections.hpp"
#include "package.hpp"

// Envia uma sequência de pacotes PackageFileList
int send_file_list(int socket, std::vector<File> files)
{
    std::vector<char> fileContentBuffer;

    // Nâo há arquivos
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

int send_active_connections_list_all(ActiveConnections_t activeConnections)
{
    for (size_t i = 0; i < activeConnections.backups.size(); i++) {
        if (send_active_connections_list(activeConnections.backups[i].socket_id, activeConnections)) {
            return -1;
        }
    }
    return 0;
}

// Envia uma sequência de pacotes PackageActiveConnectionsList 
int send_active_connections_list(int socket, ActiveConnections_t activeConnections)
{
    std::vector<char> fileContentBuffer;
    
    // Enviar clientes
    if (activeConnections.clients.size() == 0) { // se a lista de clientes estiver 
        Package package = Package(PackageActiveConnectionsList(0, 0, true, Connection_t()));
        if (write_package_to_socket(socket, package, fileContentBuffer)) {
            return -1;
        }
    } else {
        for (size_t i = 0; i < activeConnections.clients.size(); i++) {
            Package package = Package(PackageActiveConnectionsList(activeConnections.clients.size(), i+1, true, activeConnections.clients[i]));
            if (write_package_to_socket(socket, package, fileContentBuffer)) {
                return -1;
            }
        }
    }
    // Enviar backups
    if (activeConnections.backups.size() == 0) { // se a lista de backups estiver vazia
        Package package = Package(PackageActiveConnectionsList(0, 0, false, Connection_t()));
        if (write_package_to_socket(socket, package, fileContentBuffer)) {
            return -1;
        }
    } else {
        for (size_t i = 0; i < activeConnections.backups.size(); i++) {
            Package package = Package(PackageActiveConnectionsList(activeConnections.backups.size(), i+1, false, activeConnections.backups[i]));
            if (write_package_to_socket(socket, package, fileContentBuffer)) {
                return -1;
            }
        }
    }
    return 0;
}

// Faz um ping do servidor de backup para o servidor principal
int send_ping_to_main(int socket)
{
    std::vector<char> fileContentBuffer;
    Package package = Package(PackageReplicaManagerPing());
    return write_package_to_socket(socket, package, fileContentBuffer);
}

// Retorna um ACK do servidor principal para um backup
int send_backup_ack(int socket)
{
    std::vector<char> fileContentBuffer;
    Package package = Package(PackageReplicaManagerPingResponse());
    return write_package_to_socket(socket, package, fileContentBuffer);
}

// Envia uma sequência de pacotes PackageFileContent contendo o conteúdo em path
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

    // Obtém tamanho do arquivo
    auto fsize = st.st_size;
    auto to_read = fsize;

    // Buffer de leitura
    std::vector<char> buffer(std::min((long)MAX_DATA_SIZE, to_read));

    // Arquivos vazios não seriam enviado se a condição para o for fosse to_read > 0, para enviar
    //   arquivos sem conteúdo a verificação foi movida para o final do loop
    for (auto seqn = 1;; seqn++)
    {
        // Lê arquivo
        size_t to_read_now = std::min((long)MAX_DATA_SIZE, to_read);
        auto read = fread(buffer.data(), sizeof(char), to_read_now, fp);

        if (to_read_now != read)
        {
            goto fail;
        }

        to_read -= read;

        // Cria pacote a ser enviado com o conteúdo recém lido
        package = Package(PackageFileContent(fsize, seqn, read));

        if (write_package_to_socket(socket, package, buffer))
        {
            goto fail;
        }

        if (to_read <= 0)
        {
            break;
        }
    }

    fclose(fp);

    return 0;

// Fecha arquivo e retorna erro
fail:
    fclose(fp);

    return -1;
}

// Envia um arquivo em path com o nome de filename, enviará o pacote PackageUploadFile ou
//   PackageNotFound, caso PackageUploadFile tenha sido enviado chamará send_file para enviar o
//   conteúdo do arquivo
int send_file_from_path(int socket_id, const char *path, const char *filename)
{
    Package package;
    std::vector<char> fileContentBuffer;

    // Obtém File()
    auto file_opt = file_from_path(path, filename);

    if (!file_opt.has_value())
    {
        // Arquivo não existe, será indicado
        package = Package(PackageFileNotFound());
    }
    else
    {
        // Arquivo exite, será enviado o File() obtido
        package = Package(PackageUploadFile(file_opt.value()));
    }

    if (write_package_to_socket(socket_id, package, fileContentBuffer))
    {
        return 1;
    }

    if (!file_opt.has_value())
    {
        return 0;
    }

    // Envia arquivo
    return send_file(socket_id, path);
}

// Lê uma sequência de pacotes PackageFileList e retorna um vetor contendo os File's recebidos
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

        // Tipo inesperado
        if (package.package_type != FILE_LIST)
        {
            return std::nullopt;
        }

        // Se for o primeiro pacote obtemos a informação sobre quantos arquivos há
        if (first_package)
        {
            total_files = package.package_specific.fileList.count;
            first_package = false;

            // Não há arquivos, retornamos um vetor vazio
            if (total_files == 0)
            {
                return std::vector<File>(0);
            }
        }

        // Adiciona file
        files.push_back(package.package_specific.fileList.file);

        // Todos arquivos foram obtidos
        if (total_files == files.size())
        {
            break;
        }
    }

    return files;
}

// envia um arquivo para a lista de backups
int send_file_to_backups(ActiveConnections_t activeConnections, const char *path, const char *filename)
{  
    for (Connection_t backup : activeConnections.backups) {
        if (send_file_from_path(backup.socket_id, path, filename)) {
            return 1;
        }
    }
    return 0;
}

// Lê uma sequência de pacotes PackageFileContent e armazena o conteúdo obtido em path
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

        // Tipo de pacote inesperado
        if (package.package_type != FILE_CONTENT)
        {
            goto fail;
        }

        // Se for o primeiro pacote obtemos a informação sobre o tamanho do arquivo
        if (first_package)
        {
            fsize = package.package_specific.fileContent.size;
            first_package = false;

            // Arquivo não tem conteúdo
            if (fsize == 0)
            {
                break;
            }
        }

        // Escreve conteúdo obtido
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

        // Escreveu todo conteúdo que deveria
        if (total_write == fsize)
        {
            break;
        }
    }

    fclose(fp);

    return 0;

// Fecha arquivo e retorna erro
fail:
    fclose(fp);

    return -1;
}

// Primeiramente lê um pacote e verifica seu tipo, se PackageFileNotFound não então haverá alterações,
//   caso PackageUploadFile read_file_and_save será chamado
int read_upload_file_and_save(int socket, const char *path)
{
    Package package;
    std::vector<char> fileContentBuffer;

    if (read_package_from_socket(socket, package, fileContentBuffer))
    {
        return 1;
    }

    switch (package.package_type)
    {
    case FILE_NOT_FOUND:
        // Não receberá conteúdo pois arquivo não existe
        return 0;
    case UPLOAD_FILE:
        // Receberá conteúdo, armazena em path
        return read_file_and_save(socket, path);
    default:
        // Tipo de pacote inválido
        return 1;
    }
}
