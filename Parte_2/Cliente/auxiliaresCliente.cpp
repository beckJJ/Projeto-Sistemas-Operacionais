#include "auxiliaresCliente.hpp"
#include <cstddef>
#include <stdio.h>
#include <ctime>
#include "../Common/package_functions.hpp"
#include "comunicacaoCliente.hpp"
#include <string>
#include "../Common/defines.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_file.hpp"
#include <ftw.h>
#include <iostream>

// Formata time_t usando strftime
void format_time(char *buffer, time_t time);

// Callback para nftw, remove fpath
int nftw_callback(const char *fpath, const struct stat *, int, struct FTW *);

/* Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido. */
int verificaParametros(char *comando, int quantidade_parametros)
{
    int numPalavras = 0;
    int dentroPalavra = 0;

    for (int i = 0; comando[i] != '\0'; i++)
    {
        if (comando[i] == ' ' || comando[i] == '\t')
            dentroPalavra = 0;

        else
        {
            if (dentroPalavra == 0)
            {
                numPalavras++;
                dentroPalavra = 1;
            }
        }
    }

    return (numPalavras == quantidade_parametros + 1);
}

/* Retorna um vetor de char * para o inicio de cada string do comando */
std::vector<char *> splitComando(char *comando)
{
    std::vector<char *> strings;
    bool dentroPalavra = false;

    for (size_t i = 0; comando[i] != '\0'; i++)
    {
        if (comando[i] == ' ' || comando[i] == '\t')
        {
            // Estava dentro de uma palavra, \0 para indicar fim
            if (dentroPalavra)
            {
                comando[i] = '\0';
            }

            dentroPalavra = false;
        }
        else
        {
            // Encontrou nova palavra, adiciona ao vetor
            if (!dentroPalavra)
            {
                strings.push_back(&comando[i]);
                dentroPalavra = true;
            }
        }
    }

    return strings;
}

// Formata time_t usando strftime
void format_time(char *buffer, time_t time)
{
    auto tmp = localtime(&time);

    if (!tmp)
    {
        return;
    }

    strftime(buffer, sizeof(char) * TIME_FORMATTED_BUFFER, TIME_FORMAT, tmp);
}

// Imprime uma lista de File's
void print_files(std::vector<File> files)
{
    char mtime_formatted[TIME_FORMATTED_BUFFER]{};
    char atime_formatted[TIME_FORMATTED_BUFFER]{};
    char ctime_formatted[TIME_FORMATTED_BUFFER]{};

    printf(HEADER_FORMAT, "Nome", "Tamanho", "M time", "A time", "C time");

    for (auto file : files)
    {
        format_time(mtime_formatted, file.mtime);
        format_time(atime_formatted, file.atime);
        format_time(ctime_formatted, file.ctime);
        printf(FILE_FORMAT, file.name, file.size, mtime_formatted, atime_formatted, ctime_formatted);
    }
}

// Faz requisição para conectar dispositivo, dadosConexao terá campo correspondente alterado
int conecta_device(DadosConexao &dadosConexao)
{
    // Obtém socket
    auto socket_opt = conecta_servidor(dadosConexao);

    if (!socket_opt.has_value())
    {
        return 1;
    }

    int current_socket = socket_opt.value();

    dadosConexao.socket = current_socket;

    // Envia pacote inicial de identificação, especificando dispositivo
    auto package = Package(PackageUserIndentification(
        dadosConexao.deviceID,
        dadosConexao.nome_usuario));
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(current_socket, package, fileContentBuffer))
    {
        printf("Nao foi possivel enviar nome de usuario.\n");
        return 1;
    }

    if (read_package_from_socket(current_socket, package, fileContentBuffer))
    {
        printf("Erro ao ler resposta inicial do servidor.\n");
        return 1;
    }

    // Resposta inválida
    if (package.package_type != USER_INDENTIFICATION_RESPONSE)
    {
        printf("Resposta invalida do servidor.\n");
        return 1;
    }

    // Dispositivo rejeitado
    if (package.package_specific.userIdentificationResponse.status == REJECTED)
    {
        printf("Nao foi possivel se registrar como dispositivo para usuario \"%s\".\n", dadosConexao.nome_usuario);
        return 1;
    }

    dadosConexao.deviceID = package.package_specific.userIdentificationResponse.deviceID;

    return 0;
}

// Remove sync_dir_user local e baixa os arquivos presentes no servidor
// Requisitará a lista de arquivos do servidor, então para cada arquivo dessa lista o arquivo será
//   requisitado e salvo em sync_dir
int get_sync_dir(DadosConexao &dadosConexao)
{
    struct stat st;
    std::string sync_dir_path = PREFIXO_DIRETORIO;
    sync_dir_path.append(dadosConexao.nome_usuario);

    // Remove sync dir caso exista
    if (stat(sync_dir_path.c_str(), &st) != -1)
    {
        // Deve ser removido recursivamente já que pode existir arquivos
        if (remove_path_tree(sync_dir_path.c_str()))
        {
            printf("Nao foi possivel remover sync_dir local.\n");
            return 1;
        }
    }

    // Cria sync dir novamente
    if (create_dir_if_not_exists(sync_dir_path.c_str()))
    {
        printf("Erro criar diretorio sync dir do usuario.\n");
        return 1;
    }

    // Pede lista de arquivos do servidor
    auto package = Package(PackageRequestFileList());
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(dadosConexao.socket, package, fileContentBuffer))
    {
        printf("Erro ao pedir lista de arquivos ao servidor.\n");
        return 1;
    }

    auto file_list = read_file_list(dadosConexao.socket);

    if (!file_list.has_value())
    {
        printf("Erro ao ler lista de arquivos do servidor.\n");
        return 1;
    }

    // Baixa todos os arquivos que estavam na lista de arquivos do servidor
    for (auto file : file_list.value())
    {
        std::string file_path = sync_dir_path;
        file_path.append("/");
        file_path.append(file.name);

        // Pede arquivo do servidor e salva em sync_dir
        switch (download_file(dadosConexao.socket, file.name, file_path.c_str()))
        {
        case SUCCESS:
        case DOWNLOAD_FILE_FILE_NOT_FOUND: // Pode ter sido removido por outro dispostivo
            break;
        case DOWNLOAD_FILE_ERROR:
        default:
            printf("Erro ao receber arquivo \"%s\" do servidor.\n", file.name);
            return 1;
        }
    }

    return 0;
}

// Remove path e seus filhos recursivamente
int remove_path_tree(const char *path)
{
    return nftw(path, nftw_callback, 64, FTW_DEPTH);
}

// Callback para nftw, remove fpath
int nftw_callback(const char *fpath, const struct stat *, int, struct FTW *)
{
    int rv = remove(fpath);

    if (rv)
    {
        perror(fpath);
    }

    return rv;
}

// Envia requisição para baixar arquivo e o salva em out_path
int download_file(int socket_id, const char *filename, const char *out_path)
{
    Package package = Package(PackageRequestFile(filename));
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(socket_id, package, fileContentBuffer))
    {
        return DOWNLOAD_FILE_ERROR;
    }

    if (read_package_from_socket(socket_id, package, fileContentBuffer))
    {
        return DOWNLOAD_FILE_ERROR;
    }

    switch (package.package_type)
    {
    case FILE_NOT_FOUND:
        return DOWNLOAD_FILE_FILE_NOT_FOUND;
    case UPLOAD_FILE:
        if (read_file_and_save(socket_id, out_path))
        {
            return DOWNLOAD_FILE_ERROR;
        }

        return SUCCESS;
    default:
        return DOWNLOAD_FILE_ERROR;
    }
}
