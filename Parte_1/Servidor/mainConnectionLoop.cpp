#include "mainConnectionLoop.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_file.hpp"

extern DeviceManager deviceManager;

void mainConnectionLoop(int socket_id, int tid, std::string &username, User *&user)
{
    auto package = Package();
    std::vector<char> fileContentBuffer;

    std::string path_base = std::string(PREFIXO_DIRETORIO_SERVIDOR);
    path_base.append("/");
    path_base.append(username);
    path_base.append("/");

    while (true)
    {
        if (read_package_from_socket(socket_id, package, fileContentBuffer))
        {
            printf("[main] [tid: %d] Nao foi possivel ler pacote.\n", tid);
            break;
        }

        // A lock user->files_lock é obtida para evitar que os arquivos sejam modificados enquanto
        //   estão sendo acessados
        switch (package.package_type)
        {
        case REQUEST_FILE:
        {
            std::string path = path_base;
            path.append(package.package_specific.requestFile.filename);

            // Envia arquivo para o usuário
            pthread_mutex_lock(user->files_lock);
            send_file_from_path(socket_id, path.c_str(), package.package_specific.requestFile.filename);
            pthread_mutex_unlock(user->files_lock);

            break;
        }
        case REQUEST_FILE_LIST:
        {
            // Envia lista de arquivos para o usuário
            pthread_mutex_lock(user->files_lock);
            send_file_list(socket_id, *user->files);
            pthread_mutex_unlock(user->files_lock);
            break;
        }
        case UPLOAD_FILE:
        {
            std::string path = path_base;
            path.append(package.package_specific.uploadFile.name);

            pthread_mutex_lock(user->files_lock);

            // Lê arquivo que o usuário está fazendo upload
            if (read_file_and_save(socket_id, path.c_str()))
            {
                printf("[main] [tid: %d] Nao foi possivel receber arquivo enviado pelo usuario.\n", tid);
            }
            else
            {
                user->add_file_or_replace(package.package_specific.uploadFile);
            }

            pthread_mutex_unlock(user->files_lock);

            break;
        }
        default:
            printf("[main] [tid: %d] Codigo comunicacao desconhecido: 0x%02x\n", tid, (uint8_t)package.package_type);
            break;
        }
    }
}
