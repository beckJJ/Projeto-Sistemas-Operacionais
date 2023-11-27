#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include "../Common/defines.hpp"
#include "../Common/package.hpp"
#include <utility>
#include <map>
#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <optional>

struct User;

// Dados retornados ao conectar-se em DeviceManager::connect_*
struct DeviceConnectReturn
{
    // Informações do usuário, é retornado para acessar locks e informações que devem ser
    //   compartilhadas entre dispositivos distintos
    User *user;
    // Tipo da conexão
    ConnectionType connectionType;
    // ID do dispositivo conectado
    uint8_t deviceID;

    DeviceConnectReturn(User *user, ConnectionType connectionType, uint8_t deviceID);
};

// Representa um dispositivo do usuário
struct Device
{
    // Socket da comunicação principal
    int main_socket;
    // Socket da comunicação de eventos
    int event_socket;
    // ID do dispositivo
    uint8_t deviceID;

    // Termina conexão dos sockets
    void close_sockets(void);
};

// Representação um usuário, contém informação sobre seus arquivos e dispositivos
struct User
{
    // Lock para alterar os dispositivos conectados
    pthread_mutex_t *devices_lock;
    // Lock para alterar os arquivos do usuário
    pthread_mutex_t *files_lock;
    // Arquivos do usuário
    std::vector<File> *files;
    // Dispositivos atualmente conectados
    std::vector<Device> *devices;
    // Para evitar um loop de propagação de eventos, verificamos qual foi o último eventos recebido,
    //   caso o atual seja o mesmo do anterior com deviceID diferente, então o evento foi propagado
    //   para outro dispositivo e esse evento deve ser ignorado
    PackageChangeEvent previousPackageChangeEvent;

    User();
    ~User();

    // Os métodos assumem que as locks já tenham sido obtidas

    // Iniciliza files com a listagem de sync_dir_SERVER/<username>
    int initialize_files(const char *username);

    // Operações em files, sync_dir deverá ser alterado por quem chamar as funções abaixo
    std::optional<File> get_file(const char filename[NAME_MAX]);
    void create_file(const char filename[NAME_MAX]);
    void rename_file(const char old_filename[NAME_MAX], const char new_filename[NAME_MAX]);
    void remove_file(const char filename[NAME_MAX]);
    void add_file_or_replace(File file);
    // Atualiza informação de filename com base nas informações em path, chamará add_file_or_replace
    void update_file_info(const char *path, const char filename[NAME_MAX]);

    // Propaga evento para dispositivos com dispositivos com ID diferente de packageChangeEvent.deviceID
    void propagate_event(PackageChangeEvent packageChangeEvent);
};

// Gerenciador de dispositivos
class DeviceManager
{
private:
    // Hash nome de usuario para User
    std::map<std::string, User *> usuarios;
    pthread_mutex_t usuarios_lock = PTHREAD_MUTEX_INITIALIZER;

public:
    DeviceManager();
    ~DeviceManager();

    // Conecta thread como conexão principal
    std::optional<DeviceConnectReturn> connect_main(int socket_id, std::string &user);
    // Conecta thread como conexão de eventos
    std::optional<DeviceConnectReturn> connect_event(int socket_id, std::string &user, uint8_t deviceID);

    // Desconecta determinado dispositivo de um usuário, os sockets serão fechados por
    //   device.close_sockets()
    void disconnect(std::string &user, uint8_t id);

    // Desconecta todas os usuários
    void disconnect_all(void);
};

#endif
