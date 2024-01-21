#include "deviceManager.hpp"
#include <signal.h>
#include <new>
#include "../Common/functions.hpp"
#include <string.h>
#include "../Common/package_functions.hpp"
#include <algorithm>
#include "../Common/package_file.hpp"
#include <arpa/inet.h>

extern ActiveConnections_t activeConnections;

DeviceConnectReturn::DeviceConnectReturn(Device *device, User *user, uint8_t deviceID)
    : device(device), user(user), deviceID(deviceID) {}

Device::Device()
{
    socket = -1;
    socket_lock = new pthread_mutex_t;

    pthread_mutex_init(socket_lock, NULL);
}

Device::~Device()
{
    close_sockets();

    pthread_mutex_destroy(socket_lock);

    delete socket_lock;
}

// Termina conexão dos sockets
void Device::close_sockets(void)
{
    if (socket != -1)
    {
        close(socket);
        socket = -1;
    }
}

User::User()
{
    previousPackageChangeEvent = PackageChangeEvent((ChangeEvents)0xff, (uint8_t)0xff, "", "");

    files = new std::vector<File>();
    devices = new std::vector<Device *>();

    devices_lock = new pthread_mutex_t;
    files_lock = new pthread_mutex_t;

    pthread_mutex_init(devices_lock, NULL);
    pthread_mutex_init(files_lock, NULL);
}

User::~User()
{
    pthread_mutex_destroy(devices_lock);
    pthread_mutex_destroy(files_lock);

    for (auto device : *devices)
    {
        delete device;
    }

    delete devices;
    delete files;
    delete devices_lock;
    delete files_lock;
}

// Iniciliza campos que não foram possíveis inicializar no contrutos (files e username)
int User::init(const char *username)
{
    std::string path = PREFIXO_DIRETORIO_SERVIDOR;
    path.append("/");
    path.append(username);

    // Copia nome do usuário
    strncpy(this->username, username, USER_NAME_MAX_LENGTH - 1);

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
    if (!get_file(old_filename).has_value())
    {
        return;
    }

    if (get_file(new_filename).has_value())
    {
        remove_file(new_filename);
    }

    for (size_t index = 0; index < (*files).size(); index++)
    {
        if (!strcmp((*files)[index].name, old_filename))
        {
            strncpy((*files)[index].name, new_filename, NAME_MAX - 1);
            remove_file(new_filename);
            break;
        }
    }
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
    std::string path = PREFIXO_DIRETORIO_SERVIDOR;
    path.append("/");
    path.append(username);
    path.append("/");
    path.append(package.package_specific.changeEvent.filename1);

    for (auto userDevice : *devices)
    {
        // Não envia evento para dispositivo que originou o evento
        if (userDevice->deviceID == packageChangeEvent.deviceID)
        {
            continue;
        }

        // Conexão de eventos ainda não foi estabelecida
        if (userDevice->socket == -1)
        {
            continue;
        }

        pthread_mutex_lock(userDevice->socket_lock);

        // Envia evento
        if (write_package_to_socket(userDevice->socket, package, fileContentBuffer))
        {
            printf("Erro ao propagar evento para dispositivo com ID 0%02x.\n", (uint8_t)userDevice->deviceID);
        }

        // Envia conteúdo do arquivo caso o evento seja FILE_MODIFIED
        if (package.package_specific.changeEvent.event == FILE_MODIFIED)
        {
            // Envia o arquivo modificado
            if (send_file_from_path(userDevice->socket,
                                    path.c_str(),
                                    package.package_specific.changeEvent.filename1))
            {
                printf("Erro ao enviar conteudo do arquivo \"%s\" modificado.\n",
                       package.package_specific.changeEvent.filename1);
            }
        }

        pthread_mutex_unlock(userDevice->socket_lock);
    }
}

DeviceManager::DeviceManager() {}

DeviceManager::~DeviceManager()
{
    // Desconecta todos usuários
    disconnect_all();
}

std::optional<DeviceConnectReturn> DeviceManager::connect(Connection_t connection, std::string &username)
{
    if (username == "backup") {
        return connectBackup(connection);
    }
    else {
        return connectClient(connection, username);
    }
}

// Conecta thread do servidor principal com servidor de backup
std::optional<DeviceConnectReturn> DeviceManager::connectBackup(Connection_t backup)
{
    uint8_t deviceID;
    
    // encontrar primeiro valor livre de deviceID
    pthread_mutex_lock(&backups_lock);
    deviceID = nextBackupID;
    backups.push_back(deviceID);
    nextBackupID++;
    pthread_mutex_unlock(&backups_lock);

    // adicionar cliente na lista de clientes conectados
    pthread_mutex_lock(activeConnections.lock);

    activeConnections.backups.push_back(backup);

    printf("Clientes conectados:\n");
    for (Connection_t c : activeConnections.clients) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(c.address.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("%s\t", clientIP);
        printf("%d\t", ntohs(c.address.sin_port));
        printf("%d\n", c.socket_id);
    }
    printf("Backups conectados:\n");
    for (Connection_t c : activeConnections.backups) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(c.address.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("%s\t", clientIP);
        printf("%d\t", ntohs(c.address.sin_port));
        printf("%d\n", c.socket_id);
    }
    printf("\n");
    
    pthread_mutex_unlock(activeConnections.lock);

    return DeviceConnectReturn(NULL, NULL, deviceID);
}


// Conecta thread com dispositivo de determinado usuário
std::optional<DeviceConnectReturn> DeviceManager::connectClient(Connection_t client, std::string &user)
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

        // Inicializa campos
        if (usuario->init(user.c_str()))
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
        if (usuario->devices->at(0)->deviceID == UINT8_MAX)
        {
            deviceID = 0;
        }
        else
        {
            deviceID = usuario->devices->at(0)->deviceID + 1;
        }
    }
    else
    {
        // Não há dispositivos conectados, assume id 0
        deviceID = 0;
    }

    // Registra dispositivo
    Device *device = new Device();
    device->deviceID = deviceID;
    device->socket = client.socket_id;
    usuario->devices->push_back(device);

    pthread_mutex_unlock(usuario->devices_lock);

    // adicionar cliente na lista de clientes conectados
    pthread_mutex_lock(activeConnections.lock);
    activeConnections.clients.push_back(client);
    printf("Clientes conectados:\n");
    for (Connection_t c : activeConnections.clients) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(c.address.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("%s\t", clientIP);
        printf("%d\t", ntohs(c.address.sin_port));
        printf("%d\n", c.socket_id);
    }
    printf("\n");
    pthread_mutex_unlock(activeConnections.lock);


    return DeviceConnectReturn(device, usuario, deviceID);
}

void DeviceManager::disconnect(std::string &username, uint8_t id, Connection_t connection)
{
    if (username == "backup") {
        disconnectBackup(id, connection);
    } else {
        disconnectClient(username, id, connection);
    }
}

void DeviceManager::disconnectBackup(uint8_t id, Connection_t backup)
{
    // Remove id do backup da lista de backups
    pthread_mutex_lock(&backups_lock);
    for (int i = 0; i < (int)backups.size(); i++) {
        if (backups[i] == id) {
            backups.erase(backups.begin()+i);
            break;
        }
    }
    pthread_mutex_unlock(&backups_lock);

    // remove backup da lista de backups conectados
    pthread_mutex_lock(activeConnections.lock);
    int i = 0;
    while (backup.socket_id != activeConnections.backups[i].socket_id && i < (int)activeConnections.backups.size()) {
        i++;
    }
    if (i >= (int)activeConnections.backups.size()) {
        // deu algum erro
        printf("Erro ao tentar remover backup que nao estava conectado da lista de clientes conectados!\n");
    } else {
        activeConnections.backups.erase(activeConnections.backups.begin()+i);
    }

    printf("Clientes conectados:\n");
    for (Connection_t c : activeConnections.clients) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(c.address.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("%s\t", clientIP);
        printf("%d\t", ntohs(c.address.sin_port));
        printf("%d\n", c.socket_id);
    }
    printf("Backups conectados:\n");
    for (Connection_t c : activeConnections.backups) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(c.address.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("%s\t", clientIP);
        printf("%d\t", ntohs(c.address.sin_port));
        printf("%d\n", c.socket_id);
    }
    
    printf("\n");

    pthread_mutex_unlock(activeConnections.lock);
}

// Desconecta determinado dispositivo de um usuário, os sockets serão fechados por
//   device.close_sockets()
void DeviceManager::disconnectClient(std::string &user, uint8_t id, Connection_t client)
{
    // Evita alteração enquanto lê
    pthread_mutex_lock(&usuarios_lock);
    auto usuario = usuarios[user];
    pthread_mutex_unlock(&usuarios_lock);

    if (usuario == nullptr)
    {
        return;
    }

    Device *device;
    size_t index;
    bool found = false;

    // Vetor de dispositivos será lido e possivelmente alterado
    pthread_mutex_lock(usuario->devices_lock);

    for (index = 0; index < usuario->devices->size(); index++)
    {
        if (usuario->devices->at(index)->deviceID == id)
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
    device->close_sockets();
    delete device;

    pthread_mutex_unlock(usuario->devices_lock);

    // remove cliente da lista de clientes conectados
    pthread_mutex_lock(activeConnections.lock);

    int i = 0;
    while (client.socket_id != activeConnections.clients[i].socket_id && i < (int)activeConnections.clients.size()) {
        i++;
    }
    if (i >= (int)activeConnections.clients.size()) {
        // deu algum erro
        printf("Erro ao tentar remover cliente que nao estava conectado da lista de clientes conectados!\n");
    } else {
        activeConnections.clients.erase(activeConnections.clients.begin()+i);
    }

    printf("Clientes conectados:\n");
    for (Connection_t c : activeConnections.clients) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(c.address.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("%s\t", clientIP);
        printf("%d\t", ntohs(c.address.sin_port));
        printf("%d\n", c.socket_id);

    }
    printf("\n");

    pthread_mutex_unlock(activeConnections.lock);
}

// Desconecta todas os usuários
void DeviceManager::disconnect_all(void)
{
    // usuarios será alterado
    pthread_mutex_lock(&usuarios_lock);

    for (auto usuario : usuarios)
    {
        // User foi dinamicamente alocado
        delete usuario.second;
    }

    // Remove todas as entradas do map
    usuarios.clear();

    pthread_mutex_unlock(&usuarios_lock);
}
