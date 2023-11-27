#ifndef _DADO_CONEXAO_H_
#define _DADO_CONEXAO_H_

// Dados sobre a conexão, threads e locks

#define DIMENSAO_GERAL 100
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50

#include <pthread.h>
#include <cstdint>
#include <optional>

/* Estrutura utilizada para conter todos os dados da conexao. */
struct DadosConexao
{
    char nome_usuario[DIMENSAO_NOME_USUARIO]; /* Armazena o nome do usuario que estabeleceu conexao com o servidor. */
    char endereco_ip[DIMENSAO_GERAL];         /* Armazena o endereco IP do servidor. */
    char numero_porta[DIMENSAO_GERAL];        /* Armazena o numero da porta, usado para estabelecer conexao cliente-servidor. */

    char comando[DIMENSAO_COMANDO]; /* Armazena o comando da vez, escolhido pelo usuario. */

    // Armazena o socket sendo utilizado para comunicacao
    int main_connection_socket;
    // Armazena o socket sendo utilizado para eventos
    int event_connection_socket;
    // Lock para o socket
    pthread_mutex_t *main_connection_socket_lock;
    // Lock para socket de eventos, será usado tanto por eventThread quanto syncThread
    pthread_mutex_t *event_connection_socket_lock;
    // Usado para cancelar a thread em execução
    std::optional<pthread_t> sync_thread;
    // Usado para cancelar a thread em execução
    std::optional<pthread_t> event_thread;
    // ID do dispositivo
    uint8_t deviceID;

    DadosConexao();
    ~DadosConexao();
};

#endif
