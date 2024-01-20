#ifndef _SERV_FUNC_H_
#define _SERV_FUNC_H_

// Thread que assumirá a requisição recém aceita, deverá então tentar registrar-se como uma conexão
//   do dispositivo de usuário, dependendo da conexão então executará eventLoop ou MainLoop

#include "deviceManager.hpp"
#include <netinet/in.h>

struct ServerThreadArg
{
    // socket que a thread usará
    int socket_id;
    sockaddr_in socket_address;
};

void *serverThread(void *arg);

#endif
