#include "deviceManager.hpp"
#include <signal.h>
#include <new>
#include "../Common/functions.hpp"
#include <string.h>

UserDevices::UserDevices()
{
    files = new std::vector<File>();
    devices = new std::vector<ThreadDeviceID>();

    devices_lock = new pthread_mutex_t;
    files_lock = new pthread_mutex_t;

    pthread_mutex_init(devices_lock, NULL);
    pthread_mutex_init(files_lock, NULL);

    files_initialized = false;
}

UserDevices::~UserDevices()
{
    pthread_mutex_destroy(devices_lock);
    pthread_mutex_destroy(files_lock);

    delete devices;
    delete files;
}

int UserDevices::initialize_files(const char *username)
{
    std::string path = PREFIXO_DIRETORIO_SERVIDOR;
    path.append("/");
    path.append(username);

    // Garante que o diretório existe
    if (create_dir_if_not_exists(path.c_str()))
    {
        return 1;
    }

    // Lista arquivos do diretório
    auto returned_files = list_dir(path.c_str());

    if (!returned_files.has_value())
    {
        return 1;
    }

    *this->files = returned_files.value();

    return 0;
}

std::optional<File> UserDevices::get_file(const char *filename)
{
    for (auto file : *files)
    {
        if (!strcmp(file.name, filename))
        {
            return file;
        }
    }

    return std::nullopt;
}

void UserDevices::remove_file(const char *filename)
{
    for (size_t index = 0; index < (*files).size(); index++)
    {
        if (!strcmp((*files)[index].name, filename))
        {
            files->erase(files->begin() + index);
            break;
        }
    }
}

void UserDevices::add_file_or_replace(File file)
{
    for (size_t index = 0; index < (*files).size(); index++)
    {
        if (!strcmp((*files)[index].name, file.name))
        {
            (*files)[index] = file;
            return;
        }
    }

    files->push_back(file);
}

DeviceManager::DeviceManager() {}

DeviceManager::~DeviceManager()
{
    disconnect_all();
}

std::optional<std::pair<UserDevices *, uint8_t>> DeviceManager::connect(std::string &user, pthread_t thread)
{
    uint8_t deviceID;

    // Precisamos garantir que não haja múltiplas tentativas de inicializar conexoes[user]
    pthread_mutex_lock(&conexoes_map_modify);

    // Dispositivos atuais do usuário
    auto usuario = conexoes[user];

    if (usuario == nullptr)
    {
        conexoes[user] = new UserDevices();
        usuario = conexoes[user];
    }

    pthread_mutex_unlock(&conexoes_map_modify);

    // Precisamos garantir que não hajam dois dispositivos tentando se registrar
    pthread_mutex_lock(usuario->devices_lock);

    // Diretório do usuário ainda não foi listado
    if (!usuario->files_initialized)
    {
        if (usuario->initialize_files(user.c_str()))
        {
            pthread_mutex_unlock(usuario->devices_lock);
            return std::nullopt;
        }

        usuario->files_initialized = true;
    }

    if (usuario->devices->size() == 2)
    {
        pthread_mutex_unlock(usuario->devices_lock);
        return std::nullopt;
    }
    else if (usuario->devices->size() == 1)
    {
        // Obtém o próximo ID
        deviceID = usuario->devices->at(0).deviceID + 1;
    }
    else
    {
        // Não há dispositivos conectados, assume id 1
        deviceID = 1;
    }

    usuario->devices->push_back({thread, deviceID});

    pthread_mutex_unlock(usuario->devices_lock);

    return std::make_pair(usuario, deviceID);
}

void DeviceManager::disconnect(std::string &user, uint8_t id)
{
    auto usuario = conexoes[user];

    pthread_mutex_lock(usuario->devices_lock);

    // Ajusta vetor se a thread sendo removida esteja no índice 0 e tem outra no índice 1
    if (usuario->devices->size() == 2 && usuario->devices->at(0).deviceID == id)
    {
        usuario->devices->at(0) = usuario->devices->at(1);
    }

    usuario->devices->pop_back();

    pthread_mutex_unlock(usuario->devices_lock);
}

void DeviceManager::disconnect_all(void)
{
    // Envia sinal SIGTERM para todas as threads
    for (auto usuario : conexoes)
    {
        for (auto device : *usuario.second->devices)
        {
            pthread_kill(device.thread, SIGTERM);
        }

        delete usuario.second;
    }

    // Remove todas as entradas
    conexoes.clear();
}
