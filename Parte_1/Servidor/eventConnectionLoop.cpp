#include "eventConnectionLoop.hpp"
#include "../Common/functions.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/package_file.hpp"

void eventConnectionLoop(int socket_id, int tid, std::string &username, User *&user)
{
    auto package = Package();
    std::vector<char> fileContentBuffer;
    std::string path_base = std::string(PREFIXO_DIRETORIO_SERVIDOR);
    path_base.append("/");
    path_base.append(username);
    path_base.append("/");
    std::string path1;
    std::string path2;
    FILE *fp;

    while (true)
    {
        bool propagate_event = true;

        path1 = path_base;
        path2 = path_base;

        if (read_package_from_socket(socket_id, package, fileContentBuffer))
        {
            printf("[event] [tid: %d] Nao foi possivel ler pacote.\n", tid);
            break;
        }

        // Ignora pacotes que não sejam CHANGE_EVENT
        if (package.package_type != CHANGE_EVENT)
        {
            printf("[event] [tid: %d] Codigo comunicacao ignorado: 0x%02x\n", tid, (uint8_t)package.package_type);
            continue;
        }

        auto changeEvent = package.package_specific.changeEvent;

        // user->files será modificado apenas se a operação for bem sucedida, então a lock será
        //   obtida apenas no caso onde a operação seja bem sucedida
        switch (changeEvent.event)
        {
        case FILE_DELETED:
            path1.append(changeEvent.filename1);

            pthread_mutex_lock(user->files_lock);

            if (!remove(path1.c_str()))
            {
                user->remove_file(changeEvent.filename1);
            }

            pthread_mutex_unlock(user->files_lock);

            break;
        case FILE_CREATED:
            path1.append(changeEvent.filename1);

            pthread_mutex_lock(user->files_lock);

            fp = fopen(path1.c_str(), "w");

            if (fp)
            {
                user->update_file_info(path1.c_str(), changeEvent.filename1);
                fclose(fp);
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
            }

            pthread_mutex_unlock(user->files_lock);

            break;
        case FILE_RENAME:
            path1.append(changeEvent.filename1);
            path2.append(changeEvent.filename2);

            pthread_mutex_lock(user->files_lock);

            if (!rename(path1.c_str(), path2.c_str()))
            {
                user->rename_file(changeEvent.filename1, changeEvent.filename2);
                user->update_file_info(path2.c_str(), changeEvent.filename2);
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
}
