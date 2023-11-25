#ifndef _SERV_FUNC_H_
#define _SERV_FUNC_H_

#include "deviceManager.hpp"

struct thread_arg_t
{
    // socket que a thread usará
    int socket_id;
};

void *servFunc(void *arg);

#endif
