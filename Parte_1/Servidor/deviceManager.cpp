#include "deviceManager.hpp"
#include <signal.h>

DeviceManager::DeviceManager() {
    global_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

DeviceManager::~DeviceManager() {
    disconnect_all();
}

int DeviceManager::connect(std::string &user, pthread_t thread)
{
    // Dispositivos atuais do usuário
    auto usuario = conexoes[user];

    pthread_mutex_lock(&usuario.lock);

    if (usuario.devices.size() == 2) {
        pthread_mutex_unlock(&usuario.lock);
        return DEVICES_FULL;
    }

    // Se não há 2 dispositivos, adiciona thread no final do vetor
    usuario.devices.push_back(thread);
    conexoes[user].devices = usuario.devices;

    pthread_mutex_unlock(&usuario.lock);

    return SUCCESS;
}

void DeviceManager::disconnect(std::string &user, pthread_t thread)
{
    auto usuario = conexoes[user];

    pthread_mutex_lock(&usuario.lock);

    // Ajusta vetor se a thread sendo removida esteja no índice 0 e tem outra no índice 1
    if (usuario.devices.size() == 2 && usuario.devices[0] == thread) {
        usuario.devices[0] = usuario.devices[1];
    }

    usuario.devices.pop_back();
    conexoes[user].devices = usuario.devices;

    pthread_mutex_unlock(&usuario.lock);
}

void DeviceManager::disconnect_all(void)
{
    pthread_mutex_lock(&global_lock);

    // Senda sinal SIGTERM para todas as threads
    for (auto usuario : conexoes) {
        for (auto thread : usuario.second.devices) {
            pthread_kill(thread, SIGTERM);
        }
    }

    // Remove todas as entradas
    conexoes.clear();

    pthread_mutex_unlock(&global_lock);
}
