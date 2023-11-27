#include "deviceManager.hpp"
#include <signal.h>
#include <new>
#include "../Common/functions.hpp"
#include <string.h>
#include "../Common/package_functions.hpp"

DeviceConnectReturn::DeviceConnectReturn(User *user, ConnectionType connectionType, uint8_t deviceID)
    : user(user), connectionType(connectionType), deviceID(deviceID) {}

// Termina conexão dos sockets
void Device::close_sockets(void)
{
    if (main_socket != -1)
    {
        close(main_socket);
        main_socket = -1;
    }

    if (event_socket != -1)
    {
        close(event_socket);
        event_socket = -1;
    }
}

User::User()
{
    previousPackageChangeEvent = PackageChangeEvent((ChangeEvents)0xff, (uint8_t)0xff, "", "");

    files = new std::vector<File>();
    devices = new std::vector<Device>();

    devices_lock = new pthread_mutex_t;
    files_lock = new pthread_mutex_t;

    pthread_mutex_init(devices_lock, NULL);
    pthread_mutex_init(files_lock, NULL);
}

User::~User()
{
    pthread_mutex_destroy(devices_lock);
    pthread_mutex_destroy(files_lock);

    delete devices;
    delete files;
    delete devices_lock;
    delete files_lock;
}

// Iniciliza files com a listagem de sync_dir_SERVER/<username>
int User::initialize_files(const char *username)
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

std::optional<File> User::get_file(const char filename[NAME_MAX])
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

void User::create_file(const char filename[NAME_MAX])
{
    files->push_back(File(0, 0, 0, 0, filename));
}

void User::rename_file(const char old_filename[NAME_MAX], const char new_filename[NAME_MAX])
{
    for (size_t index = 0; index < (*files).size(); index++)
    {
        if (!strcmp((*files)[index].name, old_filename))
        {
            strncpy((*files)[index].name, new_filename, NAME_MAX - 1);
            break;
        }
    }

    // Pode existir um arquivo com o nome antigo
    remove_file(old_filename);
}

void User::remove_file(const char filename[NAME_MAX])
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

void User::add_file_or_replace(File file)
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

void User::update_file_info(const char *path, const char filename[NAME_MAX])
{
    auto file_opt = file_from_path(path, filename);

    if (!file_opt.has_value())
    {
        return;
    }

    add_file_or_replace(file_opt.value());
}

// Propaga evento para dispositivos com dispositivos com ID diferente de packageChangeEvent.deviceID
void User::propagate_event(PackageChangeEvent packageChangeEvent)
{
    auto package = Package(packageChangeEvent);
    std::vector<char> fileContentBuffer;

    for (auto userDevice : *devices)
    {
        // Não envia evento para dispositivo que originou o evento
        if (userDevice.deviceID == packageChangeEvent.deviceID)
        {
            continue;
        }

        // Conexão de eventos ainda não foi estabelecida
        if (userDevice.event_socket == -1)
        {
            continue;
        }

        // Envia evento
        if (write_package_to_socket(userDevice.event_socket, package, fileContentBuffer))
        {
            printf("Erro ao propagar evento para dispositivo com ID 0%02x.\n", (uint8_t)userDevice.deviceID);
        }
    }
}

DeviceManager::DeviceManager() {}

DeviceManager::~DeviceManager()
{
    // Desconecta todos usuários
    disconnect_all();
}

// Conecta thread como conexão principal
std::optional<DeviceConnectReturn> DeviceManager::connect_main(int socket_id, std::string &user)
{
    uint8_t deviceID;

    // Precisamos garantir que não haja múltiplas tentativas de inicializar usuarios[user]
    pthread_mutex_lock(&usuarios_lock);

    // Dispositivos atuais do usuário
    auto usuario = usuarios[user];

    // std::map inicializa ponteiros com nullptr
    if (usuario == nullptr)
    {
        usuarios[user] = new User();
        usuario = usuarios[user];

        // Inicializa lista de arquivos do usuário
        if (usuario->initialize_files(user.c_str()))
        {
            // Não foi possível ler arquivos do usuário, remove usuario
            delete usuario;
            usuarios.erase(user);
            pthread_mutex_unlock(&usuarios_lock);
            return std::nullopt;
        }
    }

    pthread_mutex_unlock(&usuarios_lock);

    // Precisamos garantir que não hajam dois dispositivos tentando se registrar
    pthread_mutex_lock(usuario->devices_lock);

    // Já existem dois dispositivos conectados, rejeita conexão
    if (usuario->devices->size() == 2)
    {
        pthread_mutex_unlock(usuario->devices_lock);
        return std::nullopt;
    }
    else if (usuario->devices->size() == 1)
    {
        // Obtém o próximo ID, garante que não haja saturação em UINT8_MAX
        if (usuario->devices->at(0).deviceID == UINT8_MAX)
        {
            deviceID = 0;
        }
        else
        {
            deviceID = usuario->devices->at(0).deviceID + 1;
        }
    }
    else
    {
        // Não há dispositivos conectados, assume id 0
        deviceID = 0;
    }

    // Registra dispositivo
    usuario->devices->push_back({socket_id, -1, deviceID});

    pthread_mutex_unlock(usuario->devices_lock);

    return DeviceConnectReturn(usuario, MAIN_CONNECTION, deviceID);
}

// Conecta thread como conexão de eventos
std::optional<DeviceConnectReturn> DeviceManager::connect_event(int socket_id, std::string &user, uint8_t deviceID)
{
    // Evita alteração enquanto lê
    pthread_mutex_lock(&usuarios_lock);

    auto usuario = usuarios[user];

    // usuario já deveria existir, só pode ser criado em connect_main
    if (usuario == nullptr)
    {
        pthread_mutex_unlock(&usuarios_lock);
        return std::nullopt;
    }

    pthread_mutex_unlock(&usuarios_lock);

    bool found = false;

    // Precisamos garantir que não haja alteração da lista de dispositivos
    pthread_mutex_lock(usuario->devices_lock);

    // Procura pelo dispositivo por deviceID e registra como thread de eventos caso encontre
    for (size_t i = 0; i < usuario->devices->size(); i++)
    {
        if ((*usuario->devices)[i].deviceID == deviceID)
        {
            (*usuario->devices)[i].event_socket = socket_id;
            found = true;
            break;
        }
    }

    pthread_mutex_unlock(usuario->devices_lock);

    // Não encontrou dispositivo com determinado ID
    if (!found)
    {
        return std::nullopt;
    }

    return DeviceConnectReturn(usuario, EVENT_CONNECTION, deviceID);
}

// Desconecta determinado dispositivo de um usuário, os sockets serão fechados por
//   device.close_sockets()
void DeviceManager::disconnect(std::string &user, uint8_t id)
{
    // Evita alteração enquanto lê
    pthread_mutex_lock(&usuarios_lock);
    auto usuario = usuarios[user];
    pthread_mutex_unlock(&usuarios_lock);

    if (usuario == nullptr)
    {
        return;
    }

    Device device;
    size_t index;
    bool found = false;

    // Vetor de dispositivos será lido e possivelmente alterado
    pthread_mutex_lock(usuario->devices_lock);

    for (index = 0; index < usuario->devices->size(); index++)
    {
        if (usuario->devices->at(index).deviceID == id)
        {
            device = usuario->devices->at(index);
            found = true;
            break;
        }
    }

    // Não encontrou dispositivo, não altera
    if (!found)
    {
        pthread_mutex_unlock(usuario->devices_lock);
        return;
    }

    // Remove dispositivo
    usuario->devices->erase(usuario->devices->begin() + index);

    // Encerra sockets do dispositivo
    device.close_sockets();

    pthread_mutex_unlock(usuario->devices_lock);
}

// Desconecta todas os usuários
void DeviceManager::disconnect_all(void)
{
    // usuarios será alterado
    pthread_mutex_lock(&usuarios_lock);

    for (auto usuario : usuarios)
    {
        for (auto device : *usuario.second->devices)
        {
            device.close_sockets();
        }

        // User foi dinamicamente alocado
        delete usuario.second;
    }

    // Remove todas as entradas do map
    usuarios.clear();

    pthread_mutex_unlock(&usuarios_lock);
}
