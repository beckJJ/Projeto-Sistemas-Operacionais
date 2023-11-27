#ifndef _INOTIFY_THREAD_H_
#define _INOTIFY_THREAD_H_

// Thread utilizada para enviar os eventos gerados pelo inotify para o servidor

#include <cstdint>
#include <string>
#include "../Common/package.hpp"

struct UserChangeEvent
{
    ChangeEvents changeEvent;
    std::string movedFrom;
    std::string movedTo;
    // Necessário para casar eventos MOVED_TO e MOVED_FROM
    uint32_t cookie;
};

// Thread usada para enviar eventos do inotify para o servidor
void *eventThread(void *);

#endif
