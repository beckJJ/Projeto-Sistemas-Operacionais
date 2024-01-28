#ifndef _SERV_FUNC_H_
#define _SERV_FUNC_H_

// Thread que assumirá a requisição recém aceita, deverá então tentar registrar-se como uma conexão
//   do dispositivo de usuário, dependendo da conexão então executará eventLoop ou MainLoop

#include "deviceManager.hpp"
#include <netinet/in.h>

// Tamanho máximo de um hostname (ex: "localhost")
#define MAX_HOSTNAME_LENGTH 128

struct ServerThreadArg
{
    // socket que a thread usará
    int socket_id;
    uint16_t port;
    uint32_t host;
    char hostname[MAX_HOSTNAME_LENGTH];
    uint16_t listen_port;
};

//typedef struct BackupThreadArg ;

int connectToServer(Connection_t connection, std::string &username, User *&user, Device *&device, uint8_t &deviceID, bool &backupPing);
int connectBackup(Connection_t server, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer);
int connectBackupTransfer(Connection_t server, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer);
int connectUser(Connection_t client, std::string &username, User *&user, Device *&device, uint8_t &deviceID, Package &package, std::vector<char> fileContentBuffer);

void *serverThread(void *arg);

#endif
