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

struct Device;
struct User;

// Dados retornados ao conectar-se em DeviceManager::connect_*
struct DeviceConnectReturn
{
    // Dispositivo atual
    Device *device;
    // Informações do usuário, é retornado para acessar locks e informações que devem ser
    //   compartilhadas entre dispositivos distintos
    User *user;
    // ID do dispositivo conectado
    uint8_t deviceID;

    DeviceConnectReturn(Device *device, User *user, uint8_t deviceID);
};

// Representa um dispositivo do usuário
struct Device
{
    // Socket da comunicação principal
    int socket;
    // ID do dispositivo
    uint8_t deviceID;
    // Lock que deve ser adiquirida antes de enviar pacotes para o usuário
    pthread_mutex_t *socket_lock;

    Device();
    ~Device();

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
    std::vector<Device *> *devices;
    // Para evitar um loop de propagação de eventos, verificamos qual foi o último eventos recebido,
    //   caso o atual seja o mesmo do anterior com deviceID diferente, então o evento foi propagado
    //   para outro dispositivo e esse evento deve ser ignorado
    PackageChangeEvent previousPackageChangeEvent;
    // Nome do usuário
    char username[USER_NAME_MAX_LENGTH];

    User();
    ~User();

    // Os métodos assumem que as locks já tenham sido obtidas

    // Iniciliza campos que não foram possíveis inicializar no contrutos (files e username)
    int init(const char *username);

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

    // Conecta thread como dispositivo de determinado usuário
    std::optional<DeviceConnectReturn> connect(int socket_id, std::string &user);

    // Desconecta determinado dispositivo de um usuário, os sockets serão fechados por
    //   device.close_sockets()
    void disconnect(std::string &user, uint8_t id);

    // Desconecta todas os usuários
    void disconnect_all(void);
};

#endif
