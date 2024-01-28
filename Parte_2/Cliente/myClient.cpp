#include <stdio.h>
#include <string.h>
#include <iostream>
#include "../Common/package.hpp"
#include "../Common/package_functions.hpp"
#include "../Common/functions.hpp"
#include "../Common/defines.hpp"
#include "../Common/package_file.hpp"
#include "comunicacaoCliente.hpp"
#include "interfaceCliente.hpp"
#include "eventThread.hpp"
#include <unistd.h>
#include "auxiliaresCliente.hpp"
#include "readThread.hpp"
#include "frontendThread.hpp"
#include <csignal>

#if DEBUG_PACOTE
// Lock usado por print_package para exibir completamente um pacote
pthread_mutex_t print_package_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

// Dados sobre a comunicação com o servidor
DadosConexao dados_conexao;

// Último evento recebido pelo servidor, usado para determinar se o evento gerado por outro
//   dispositivo foi gerado em resposta a um evento gerado no dispositivo atual
PackageChangeEvent previousSyncedChangeEvent = PackageChangeEvent((ChangeEvents)0xff, (uint8_t)0xff, "", "", dados_conexao.nome_usuario);
// Lock para evento anterior
pthread_mutex_t previousSyncedChangeEventLock = PTHREAD_MUTEX_INITIALIZER;

// Cancela threads caso estejam executando
void cancel_threads()
{
    if (dados_conexao.sync_thread.has_value())
    {
        pthread_cancel(dados_conexao.sync_thread.value());
    }

    if (dados_conexao.event_thread.has_value())
    {
        pthread_cancel(dados_conexao.event_thread.value());
    }

    if (dados_conexao.frontend_thread.has_value()) {
        pthread_cancel(dados_conexao.frontend_thread.value());
    }
}

// Fecha sockets que estejam abertos
void close_sockets()
{
    if (dados_conexao.socket != -1)
    {
        close(dados_conexao.socket);
    }
}

// SIGINT é gerado pelo terminal ao receber Ctrl-C
void sigint_handler(int)
{
    cancel_threads();
    close_sockets();
    exit(SIGINT);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sigint_handler);

    if (argc != QUANTIDADE_PARAMETROS_MYCLIENT + 1) /* Controle do numero de correto de parametros. */
    {
        printf("Comando invalido! Numero de parametros incorreto!\n");
        printf("Forma correta:\n./myClient <username> <server_ip_address> <port>\n");
        exit(EXIT_FAILURE);
    }

    /* Copia os dados da conexao, passados como parametro pelo usuario. */
    strcpy(dados_conexao.nome_usuario, argv[1]);
    strcpy(dados_conexao.endereco_ip, argv[2]);
    strcpy(dados_conexao.numero_porta, argv[3]);

    // gera uma porta aleatória entre 5000 e 5999 para o frontend
    srand(time(NULL));
    dados_conexao.listen_port = (rand() % 1000) + 5000;
    printf("Porta gerada: %d\n", dados_conexao.listen_port);

    /* Inicia a conexao do dispositivo com o servidor. */
    if (conecta_device(dados_conexao))
    {
        exit(EXIT_FAILURE);
    }

    // Inicializa o sync dir em um estado válido.
    // Deve ser executado antes da thread de eventos ter sido inicializada
    if (get_sync_dir(dados_conexao))
    {
        exit(EXIT_FAILURE);
    }

    pthread_t new_thread;

    // Inicia thread de frontend
    pthread_create(&new_thread, NULL, frontendThread, NULL);
    dados_conexao.frontend_thread = new_thread;

    // Inicia thread de eventos
    pthread_create(&new_thread, NULL, eventThread, NULL);
    dados_conexao.event_thread = new_thread;

    // Inicia thread de leitura
    pthread_create(&new_thread, NULL, readThread, NULL);
    dados_conexao.sync_thread = new_thread;

    // A thread atual será responsável por ler comandos do usuário
    while (1)
    {
        char *ret;

        limpaTela();
        /* Exibe o menu principal ao usuario, para digitar os comandos. */
        menu_principal(dados_conexao); 
        /* Obtem o comando inserido pelo usuario. */
        ret = fgets(dados_conexao.comando, sizeof(dados_conexao.comando), stdin); 

        // Erro ou EOF
        if (ret == NULL)
        {
            break;
        }

        dados_conexao.comando[strnlen(dados_conexao.comando, sizeof(dados_conexao.comando)) - 1] = '\0';

        /* Identifica e executa o comando inserido pelo usuario. */
        if (executa_comando(dados_conexao))
        {
            // Comando exit, sai do loop e encerra
            break;
        }
    }

    cancel_threads();
    close_sockets();

    return 0;
}
