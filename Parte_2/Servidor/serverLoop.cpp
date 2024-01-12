#include "serverLoop.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_file.hpp"

extern DeviceManager deviceManager;

// Lida com pacote PackageChangeEvent
void handleChangeEvent(int socket_id, int tid, PackageChangeEvent &changeEvent, std::string &path_base, User *&user)
{
    auto package = Package();
    std::vector<char> fileContentBuffer;
    std::string path1 = path_base;
    std::string path2 = path_base;
    FILE *fp;

    bool propagate_event = false;

    // user->files será modificado apenas se a operação for bem sucedida, então a lock será
    //   obtida apenas no caso onde a operação seja bem sucedida
    switch (changeEvent.event)
    {
    case FILE_DELETED:
        path1.append(changeEvent.filename1);

        pthread_mutex_lock(user->files_lock);

        // Ignora remoção de arquivo inexistente
        if (!user->get_file(changeEvent.filename1).has_value())
        {
            pthread_mutex_unlock(user->files_lock);
            break;
        }

        if (!remove(path1.c_str()))
        {
            user->remove_file(changeEvent.filename1);
            propagate_event = true;
        }

        pthread_mutex_unlock(user->files_lock);

        break;
    case FILE_CREATED:
        path1.append(changeEvent.filename1);

        pthread_mutex_lock(user->files_lock);

        // Ignora criação caso arquivo já exista
        if (user->get_file(changeEvent.filename1).has_value())
        {
            pthread_mutex_unlock(user->files_lock);
            break;
        }

        fp = fopen(path1.c_str(), "w");

        if (fp)
        {
            user->update_file_info(path1.c_str(), changeEvent.filename1);
            fclose(fp);
            propagate_event = true;
        }

        pthread_mutex_unlock(user->files_lock);

        break;
    case FILE_MODIFIED:
        path1.append(changeEvent.filename1);

        pthread_mutex_lock(user->files_lock);

        // Após o evento FILE_MODIFIED haverá pacotes de upload de arquivo
        if (!read_upload_file_and_save(socket_id, path1.c_str()))
        {
            user->update_file_info(path1.c_str(), changeEvent.filename1);
            propagate_event = true;
        }

        pthread_mutex_unlock(user->files_lock);

        break;
    case FILE_RENAME:
        path1.append(changeEvent.filename1);
        path2.append(changeEvent.filename2);

        pthread_mutex_lock(user->files_lock);

        // Ignora renomeação de arquivo inexistente
        if (!user->get_file(changeEvent.filename1).has_value())
        {
            pthread_mutex_unlock(user->files_lock);
            break;
        }

        if (!rename(path1.c_str(), path2.c_str()))
        {
            user->rename_file(changeEvent.filename1, changeEvent.filename2);
            user->update_file_info(path2.c_str(), changeEvent.filename2);
            propagate_event = true;
        }

        pthread_mutex_unlock(user->files_lock);

        break;
    default:
        printf("[event] [tid: %d] Codigo de ChangeEvents desconhecido: 0x%02x\n", tid, (uint8_t)changeEvent.event);
        // Evento desconhecido, não propaga
        propagate_event = false;
        break;
    }

    if (propagate_event)
    {
        // Envia evento para os outros dispositivos conectados
        pthread_mutex_lock(user->devices_lock);
        user->propagate_event(changeEvent);
        pthread_mutex_unlock(user->devices_lock);
    }
}

// Loop de requisições e respotas para determinado dispositivo
void serverLoop(int socket_id, int tid, std::string &username, User *&user, Device *&device)
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
            printf("[tid: %d] Nao foi possivel ler pacote.\n", tid);
            break;
        }

        // A lock user->files_lock é obtida para evitar que os arquivos sejam modificados enquanto
        //   estão sendo acessados
        // A lock device->socket_lock é acessada enquanto pacotes forem escritos (também é
        //   adiquirida ao propagar eventos)
        switch (package.package_type)
        {
        // Envia arquivo para o usuário
        case REQUEST_FILE:
        {
            std::string path = path_base;
            path.append(package.package_specific.requestFile.filename);

            pthread_mutex_lock(user->files_lock);
            pthread_mutex_lock(device->socket_lock);
            send_file_from_path(socket_id, path.c_str(), package.package_specific.requestFile.filename);
            pthread_mutex_unlock(device->socket_lock);
            pthread_mutex_unlock(user->files_lock);

            break;
        }
        // Envia lista de arquivos para o usuário
        case REQUEST_FILE_LIST:
        {
            pthread_mutex_lock(user->files_lock);
            pthread_mutex_lock(device->socket_lock);
            send_file_list(socket_id, *user->files);
            pthread_mutex_unlock(device->socket_lock);
            pthread_mutex_unlock(user->files_lock);
            break;
        }
        // Lê arquivo que o usuário está fazendo upload e salva no sync dir do servidor
        case UPLOAD_FILE:
        {
            std::string path = path_base;
            path.append(package.package_specific.uploadFile.name);

            pthread_mutex_lock(user->files_lock);

            if (read_file_and_save(socket_id, path.c_str()))
            {
                printf("[tid: %d] Nao foi possivel receber arquivo enviado pelo usuario.\n", tid);
            }
            else
            {
                user->add_file_or_replace(package.package_specific.uploadFile);
            }

            pthread_mutex_unlock(user->files_lock);

            break;
        }
        // Pacote de evento
        case CHANGE_EVENT:
            handleChangeEvent(socket_id, tid, package.package_specific.changeEvent, path_base, user);
            break;
        default:
            printf("[tid: %d] Codigo comunicacao desconhecido: 0x%02x\n", tid, (uint8_t)package.package_type);
            break;
        }
    }
}
