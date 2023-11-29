#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

// Loop da thread responsável por receber eventos da conexão de eventos de um dispositivo

#include <string>
#include "deviceManager.hpp"

void eventConnectionLoop(int socket_id, int tid, std::string &username, User *&user, Device *&device);

#endif