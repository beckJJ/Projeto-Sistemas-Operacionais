#include <stdlib.h>
#include <sys/types.h>

#include "servFunc.hpp"
#include "../Common/structs.hpp"
#include "../Common/comunicacao.hpp"
#include <unistd.h>
#include <stdio.h>
#include "comunicacaoServidor.hpp"
#include "auxiliaresServidor.hpp"

void *servFunc(void *arg)
{
    int socket = *(int *)arg;
    pid_t tid = gettid();
    Pacote pacote;

    printf("Thread with tid %d started.\n", tid);

    while (1)
    {
        if (read(socket, &pacote, sizeof(pacote)) < 0)
        {
            printf("Erro! Nao foi possivel realizar a leitura dos dados com o socket!\n");
            break;
        }

        printf("\n[tid: %d] <<< PACOTE RECEBIDO >>>\n", tid);
        imprimeDadosPacote(pacote);

        if (pacote.codigoComunicacao == CODIGO_UPLOAD)
        {
            criaNovoDiretorio(PREFIXO_DIRETORIO_SERVIDOR, pacote.usuario);
            upload(PREFIXO_DIRETORIO_SERVIDOR, &pacote);
        }
        else if (pacote.codigoComunicacao == CODIGO_LISTSERVER)
        {
            list_server(socket, &pacote);
            printf("\n[tid: %d] <<< PACOTE ENVIADO >>>\n", tid);
            imprimeDadosPacote(pacote);
        }
        else if (pacote.codigoComunicacao == CODIGO_DOWNLOAD)
        {
            download(socket, &pacote);
            printf("\n[tid: %d] <<< PACOTE ENVIADO >>>\n", tid);
            imprimeDadosPacote(pacote);
        }
    }

    close(socket);

    return NULL;
}