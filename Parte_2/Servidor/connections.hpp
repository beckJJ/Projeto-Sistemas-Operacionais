#pragma once
#include <vector>
#include <netinet/in.h>
#include <string>

#define DIMENSAO_GERAL 100

//TODO
typedef struct {
    alignas(ALIGN_VALUE) char numero_porta[DIMENSAO_GERAL]; // buffer
    alignas(ALIGN_VALUE) char endereco_ip[DIMENSAO_GERAL]; // &buffer[104]
    alignas(ALIGN_VALUE) char username[DIMENSAO_GERAL]; // &buffer[104 * 2]
    alignas(ALIGN_VALUE) int socket_id; // &buffer[104 * 3]
} Connection_t;

// struct que guarda todas as conexões ativas do servidor principal
typedef struct {
    // Lock para alterar as conexões
    pthread_mutex_t *lock; // NOVO MUTEX

    std::vector<Connection_t> clients;
    std::vector<Connection_t> backups;
} ActiveConnections_t;
