#ifndef _SERV_FUNC_H_
#define _SERV_FUNC_H_

// Thread que assumirá a requisição recém aceita, deverá então tentar registrar-se como uma conexão
//   do dispositivo de usuário, dependendo da conexão então executará eventLoop ou MainLoop

#include "deviceManager.hpp"

struct ServerThreadArg
{
    // socket que a thread usará
    int socket_id;
};

void *serverThread(void *arg);

#endif
