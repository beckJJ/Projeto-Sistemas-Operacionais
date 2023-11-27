#ifndef _MAIN_LOOP_H_
#define _MAIN_LOOP_H_

// Loop da thread principal do dispositivo, processa os pacotes gerados pelos comandos do usuário

#include <string>
#include "deviceManager.hpp"

void mainConnectionLoop(int socket_id, int tid, std::string &username, User *&user);

#endif
