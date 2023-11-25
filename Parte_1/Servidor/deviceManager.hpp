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

#define DEVICES_FULL -1

struct ThreadDeviceID
{
    // Thread do dispositivo, usado para mandar sinais
    pthread_t thread;
    // ID do dispositivo
    uint8_t deviceID;
};

struct UserDevices
{
    // Lock para alterar o vetor de dispositivos conectados
    pthread_mutex_t *devices_lock;
    // Lock para alterar a lista de arquivos do usuário
    pthread_mutex_t *files_lock;
    // Lista de arquivos do usuário
    std::vector<File> *files;
    // Lista de dispositivos atualmente conectados
    std::vector<ThreadDeviceID> *devices;
    // Todos dispositivos que se conectam checam files_initialized, se false então lê o diretório do
    //   usuário e adiciona os arquivos em files
    bool files_initialized;

    UserDevices();
    ~UserDevices();

    int initialize_files(const char *username);
    std::optional<File> get_file(const char *filename);
    void remove_file(const char *filename);
    void add_file_or_replace(File file);
};

class DeviceManager
{
private:
    // hash usuario para threads de conexão (uma thread está conectada a um dispositivo)
    std::map<std::string, UserDevices *> conexoes;
    pthread_mutex_t conexoes_map_modify = PTHREAD_MUTEX_INITIALIZER;

public:
    DeviceManager();
    ~DeviceManager();

    // Registra a thread como dispositivo, caso sucesso o valor será um par contendo UserDevices
    //   dinamicamente alocado e o id associado a thread recém conectada
    std::optional<std::pair<UserDevices *, uint8_t>> connect(std::string &user, pthread_t thread);
    // Desconecta dispositivo com determinado ID, é enviado SIGTERM para a thread
    void disconnect(std::string &user, uint8_t id);
    // Envia SIGTERM para todos dispositivos registrados de todos usuários
    void disconnect_all(void);
};

#endif
