#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include "../Common/defines.hpp"
#include <map>
#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>

#define DEVICES_FULL    1

struct UserDevices {
    pthread_mutex_t lock;
    std::vector<pthread_t> devices;

    UserDevices() {
        std::cout << "const" << std::endl;
        lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    }
};

class DeviceManager {
private:
    // hash usuario para threads de conexão (uma thread está conectada a um dispositivo)
    std::map<std::string, UserDevices> conexoes;
    pthread_mutex_t global_lock;

public:
    DeviceManager();
    ~DeviceManager();

    int connect(std::string &user, pthread_t thread);
    void disconnect(std::string &user, pthread_t thread);
    void disconnect_all(void);
};

#endif
