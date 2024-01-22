#pragma once
#include <vector>
#include <netinet/in.h>
#include <string>

#define DIMENSAO_GERAL 100

typedef struct {
    char numero_porta[DIMENSAO_GERAL]; // Porta que o cliente/backup aguardam novas conexões
    char endereco_ip[DIMENSAO_GERAL];
    char username[DIMENSAO_GERAL];
    int socket_id;
} Connection_t;

// struct que guarda todas as conexões ativas do servidor principal
typedef struct {
    // Lock para alterar as conexões
    pthread_mutex_t *lock; // NOVO MUTEX

    std::vector<Connection_t> clients;
    std::vector<Connection_t> backups;
} ActiveConnections_t;