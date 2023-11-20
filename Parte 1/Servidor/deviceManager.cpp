#include "deviceManager.hpp"
#include <signal.h>

DeviceManager::DeviceManager() {
    // Inicializa lock
    lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

DeviceManager::~DeviceManager() {
    disconnect_all();
}

void DeviceManager::connect(std::string &user, pthread_t thread)
{
    pthread_t thread_to_kill;
    bool need_to_kill_thread = false;

    pthread_mutex_lock(&lock);

    // Dispositivos atuais do usuário
    auto usuario = conexoes[user];

    if (usuario.size() == 2) {
        // Se há dois dispositivos, remove o que está a mais tempo conectado
        thread_to_kill = usuario[0];
        need_to_kill_thread = true;

        usuario[0] = usuario[1];
        usuario[1] = thread;
    } else {
        // Se não há 2 dispositivos, adiciona thread no final do vetor
        usuario.push_back(thread);
    }

    conexoes[user] = usuario;

    pthread_mutex_unlock(&lock);

    if (need_to_kill_thread) {
        pthread_kill(thread_to_kill, SIGTERM);
    }
}

void DeviceManager::disconnect(std::string &user, pthread_t thread)
{
    pthread_mutex_lock(&lock);

    auto usuario = conexoes[user];

    // Ajusta vetor se a thread sendo removida esteja no índice 0 e tem outra no índice 1
    if (usuario.size() == 2 && usuario[0] == thread) {
        usuario[0] = usuario[1];
    }

    usuario.pop_back();
    conexoes[user] = usuario;

    pthread_mutex_unlock(&lock);
}

void DeviceManager::disconnect_all(void)
{
    pthread_mutex_lock(&lock);

    // Senda sinal SIGTERM para todas as threads
    for (auto usuario : conexoes) {
        for (auto thread : usuario.second) {
            pthread_kill(thread, SIGTERM);
        }
    }

    // Remove todas as entradas
    conexoes.clear();

    pthread_mutex_unlock(&lock);
}
