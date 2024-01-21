#pragma once
#include <vector>
#include <netinet/in.h>
#include <string>

typedef struct {
    int socket_id;
    sockaddr_in address;
} Connection_t;

// struct que guarda todas as conexões ativas do servidor principal
typedef struct {
    // Lock para alterar as conexões
    pthread_mutex_t *lock; // NOVO MUTEX

    std::vector<Connection_t> clients;
    std::vector<Connection_t> backups;
} ActiveConnections_t;