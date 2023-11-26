#ifndef _INOTIFY_THREAD_H_
#define _INOTIFY_THREAD_H_

#include <cstdint>
#include <string>
#include "../Common/package.hpp"

struct ThreadArg
{
    int socket_id;
    uint8_t device_id;
    char *username;
};

struct UserChangeEvent
{
    ChangeEvents changeEvent;
    std::string movedFrom;
    std::string movedTo;
    uint32_t cookie;
};

void *inotifyThread(void *threadArg);

#endif
