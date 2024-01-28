#ifndef _DADO_CONEXAO_H_
#define _DADO_CONEXAO_H_

// Dados sobre a conexão, threads e locks

#define DIMENSAO_GERAL 100
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50

#include <pthread.h>
#include <cstdint>
#include <optional>
#include <vector>
#include "package.hpp"

/* Estrutura utilizada para conter todos os dados da conexao. */
struct DadosConexao
{
    char nome_usuario[DIMENSAO_NOME_USUARIO]; /* Armazena o nome do usuario que estabeleceu conexao com o servidor. */
    char endereco_ip[DIMENSAO_GERAL];         /* Armazena o endereco IP do servidor. */
    char numero_porta[DIMENSAO_GERAL];        /* Armazena o numero da porta, usado para estabelecer conexao cliente-servidor. */

    char comando[DIMENSAO_COMANDO]; /* Armazena o comando da vez, escolhido pelo usuario. */

    // Armazena o socket sendo utilizado para comunicacao
    int socket;
    // Lock que deve ser adquirido antes de ler e enviar pacotes para o servidor
    pthread_mutex_t *socket_lock;
    // Condição usada para esperar leitura da lista de arquivos que deve ser feita pela thread de
    //   leitura
    pthread_cond_t *file_list_cond;
    // Lock para alterar file_list
    pthread_mutex_t *file_list_lock;
    // Deve ser verificado se já leu todo FileList
    bool is_file_list_read;
    // Lista de arquivos do servidor, atualizada sempre que PackageFileList forem recebidos
    std::vector<File> file_list;
    // Usado para cancelar a thread em execução
    std::optional<pthread_t> sync_thread;
    // Usado para cancelar a thread em execução
    std::optional<pthread_t> event_thread;
    // ID do dispositivo
    uint8_t deviceID;
    
    // Novos campos que o backup utilizará
    pthread_mutex_t *backup_connection_lock;
    bool backup_flag;
    uint16_t listen_port;
    int socket_transfer;
    int socket_ping;
    uint8_t deviceID_transfer;
    uint8_t deviceID_ping;
    std::optional<pthread_t> backup_thread;
    std::optional<pthread_t> ping_thread;


    DadosConexao();
    ~DadosConexao();
};

#endif
