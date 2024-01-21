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

int connectToServer(Connection_t connection, std::string &username, User *&user, Device *&device, uint8_t &deviceID);
int connectBackup(Connection_t server);
int connectUser(Connection_t client, std::string &username, User *&user, Device *&device, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer);

void *serverThread(void *arg);

#endif
