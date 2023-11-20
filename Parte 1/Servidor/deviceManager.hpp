#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include <map>
#include <string>
#include <vector>
#include <pthread.h>

class DeviceManager {
private:
    pthread_mutex_t lock;
    // hash usuario para threads de conexão (uma thread está conectada a um dispositivo)
    std::map<std::string, std::vector<pthread_t> > conexoes;

public:
    DeviceManager();
    ~DeviceManager();

    void connect(std::string &user, pthread_t thread);
    void disconnect(std::string &user, pthread_t thread);
    void disconnect_all(void);
};

#endif
