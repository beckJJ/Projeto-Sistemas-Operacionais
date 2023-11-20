#ifndef _SERV_FUNC_H_
#define _SERV_FUNC_H_

#include "deviceManager.hpp"

struct thread_arg_t {
    // socket que a thread usará
    int socket;
    // Será usado como argumento para registrar thread como dispositivo em
    //   DeviceManager
    pthread_t thread;
    // Genreciador de dispositivos, a thread deve ser adicionada para que possa
    //   ser interrompida, caso a execução termine normalmente a thread deve se
    //   retirar da lista de dispositivos
    DeviceManager *deviceMan;
};

void *servFunc(void *arg);

#endif
