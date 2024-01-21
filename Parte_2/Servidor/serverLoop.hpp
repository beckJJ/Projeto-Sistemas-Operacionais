#ifndef _SERVER_LOOP_H_
#define _SERVER_LOOP_H_

// Loop de requisições e respotas para determinado dispositivo

#include <string>
#include "deviceManager.hpp"

void serverLoop(int socket_id, int tid, std::string &username, User *&user, Device *&device);
void serverLoopBackup(int socket_id, int tid);
void serverLoopClient(int socket_id, int tid, std::string &username, User *&user, Device *&device);

#endif
