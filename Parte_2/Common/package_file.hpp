#ifndef _PACKAGE_FILE_H_
#define _PACKAGE_FILE_H_

// Funções de alto nível que lidam com Package de arquivos

#include "defines.hpp"
#include "package.hpp"
#include <optional>

// Envia uma sequência de pacotes PackageFileList
int send_file_list(int socket, std::vector<File> files);

// Envia uma sequência de pacotes PackageFileContent contendo o conteúdo em path
int send_file(int socket, const char *path);

// Envia um arquivo em path com o nome de filename, enviará o pacote PackageUploadFile ou
//   PackageNotFound, caso PackageUploadFile tenha sido enviado chamará send_file para enviar o
//   conteúdo do arquivo
int send_file_from_path(int socket_id, const char *path, const char *filename);

// Lê uma sequência de pacotes PackageFileList e retorna um vetor contendo os File's recebidos
std::optional<std::vector<File>> read_file_list(int socket);

// Lê uma sequência de pacotes PackageFileContent e armazena o conteúdo obtido em path
int read_file_and_save(int socket, const char *path);

// Primeiramente lê um pacote e verifica seu tipo, se PackageFileNotFound não então haverá alterações,
//   caso PackageUploadFile read_file_and_save será chamado
int read_upload_file_and_save(int socket, const char *path);

#endif
