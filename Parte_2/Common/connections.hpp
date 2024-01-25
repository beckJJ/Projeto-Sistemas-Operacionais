#pragma once
#include <vector>
#include <netinet/in.h>
#include <string>
#include "package.hpp"

// struct que guarda todas as conexões ativas do servidor principal
typedef struct
{
    // Lock para alterar as conexões
    pthread_mutex_t *lock; // NOVO MUTEX

    std::vector<Connection_t> clients;
    std::vector<Connection_t> backups;
} ActiveConnections_t;
