#pragma once
#include <vector>
#include <netinet/in.h>
#include <string>

typedef struct {
    int socket_id;
    sockaddr_in address;
} Client_t;

// struct que guarda todas as conexões ativas do servidor principal
typedef struct {
    // Lock para alterar as conexões
    pthread_mutex_t *lock;

    std::vector<Client_t> clients;
    std::vector<sockaddr_in> backups;
} ActiveConnections_t;