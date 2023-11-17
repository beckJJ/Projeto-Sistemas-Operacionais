#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "servFunc.hpp"
#include "../Common/structs.hpp"
#include "../Common/comunicacao.hpp"
#include "comunicacaoServidor.hpp"
#include "auxiliaresServidor.hpp"

// Para fechar o socket em sigterm_handler precisamos ter o socket globalmente
std::map<pid_t, int> pid_to_socket;
pthread_mutex_t pid_to_socket_lock = PTHREAD_MUTEX_INITIALIZER;

void sigterm_handler(int signal) {
    pid_t thread_tid = gettid();

    // Obtem socket que estava sendo usado pela thread
    pthread_mutex_lock(&pid_to_socket_lock);
    int socket = pid_to_socket[thread_tid];
    pid_to_socket.erase(thread_tid);
    pthread_mutex_unlock(&pid_to_socket_lock);

    printf("[tid: %d] Received SIGTERM\n", thread_tid);

    // Fecha socket
    close(socket);

    // Encerra thread
    pthread_exit(NULL);
}

void *servFunc(void *arg)
{
    int thread_socket;
    pid_t thread_tid;

    struct thread_arg_t *thread_arg = (struct thread_arg_t *)arg;

    thread_socket = thread_arg->socket;
    thread_tid = gettid();

    // Armazena socket no hashmap, poderá ser usado em sigterm_handler
    pthread_mutex_lock(&pid_to_socket_lock);
    pid_to_socket[thread_tid] = thread_socket;
    pthread_mutex_unlock(&pid_to_socket_lock);

    // Registra sigterm_handler para SIGTERM, fecha socket e informa no terminal
    signal(SIGTERM, sigterm_handler);

    // Cada pacote tem o nome de usuário, usaremos o primeiro para determinar quem o usuário é
    Pacote pacote;
    bool primeiro_pacote = true;
    std::string usuario;

    printf("[tid: %d] Thread is running.\n", thread_tid);

    while (1)
    {
        int ret;

        ret = read(thread_socket, &pacote, sizeof(pacote));

        if (ret < 0)
        {
            printf("[tid: %d] Erro! Nao foi possivel realizar a leitura dos dados com o socket!\n", thread_tid);
            break;
        }
        else if (ret == 0)
        {
            printf("[tid: %d] Cliente encerrou a conexao.\n", thread_tid);
            break;
        }

        printf("\n[tid: %d] <<< PACOTE RECEBIDO >>>\n", thread_tid);
        imprimeDadosPacote(pacote);

        if (primeiro_pacote) {
            usuario = pacote.usuario;

            // Registra thread atual como dispositivo do usuario
            thread_arg->deviceMan->connect(usuario, thread_arg->thread);
            primeiro_pacote = false;
        }

        switch (pacote.codigoComunicacao) {
        case CODIGO_UPLOAD:
            criaNovoDiretorio(PREFIXO_DIRETORIO_SERVIDOR, pacote.usuario);
            upload(PREFIXO_DIRETORIO_SERVIDOR, &pacote);
            break;
        case CODIGO_LISTSERVER:
            list_server(thread_socket, &pacote);
            printf("\n[tid: %d] <<< PACOTE ENVIADO >>>\n", thread_tid);
            imprimeDadosPacote(pacote);
            break;
        case CODIGO_DOWNLOAD:
            download(thread_socket, &pacote);
            printf("\n[tid: %d] <<< PACOTE ENVIADO >>>\n", thread_tid);
            imprimeDadosPacote(pacote);
            break;
        default:
            printf("\n[tid: %d] Codigo comunicacao desconhecido: %d\n", thread_tid, pacote.codigoComunicacao);
            break;
        }
    }

    // Fecha socket
    close(thread_socket);

    // Remove a thread da lista de dispositivos do usuário
    thread_arg->deviceMan->disconnect(usuario, thread_arg->thread);

    pthread_mutex_lock(&pid_to_socket_lock);
    pid_to_socket.erase(thread_tid);
    pthread_mutex_unlock(&pid_to_socket_lock);

    return NULL;
}
