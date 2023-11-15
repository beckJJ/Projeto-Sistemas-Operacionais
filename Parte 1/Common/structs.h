#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include "defines.h"
#include <stdint.h>

/* Estrutura utilizada para conter todos os dados da conexao. */
typedef struct
{
    char nome_usuario[DIMENSAO_NOME_USUARIO]; /* Armazena o nome do usuario que estabeleceu conexao com o servidor. */
    char endereco_ip[DIMENSAO_GERAL];         /* Armazena o endereco IP do servidor. */
    char numero_porta[DIMENSAO_GERAL];        /* Armazena o numero da porta, usado para estabelecer conexao cliente-servidor. */

    char comando[DIMENSAO_COMANDO]; /* Armazena o comando da vez, escolhido pelo usuario. */

    int socket_id; /* Armazena o valor inteiro que identifica o socket sendo utilizado para comunicacao. */

} DadosConexao;

/* Estrutura utilizada para agrupar os dados de um pacote, enviado durante a comunicacao cliente-servidor. */
typedef struct
{
    char conteudo[DIMENSAO_BUFFER];      /* Armazena o conteudo geral do pacote, podendo ser o conteudo de um arquivo ou informacoes sobre um diretorio remoto. */
    char nomePacote[DIMENSAO_GERAL];     /* Armazena o identificador do pacote, para deixar claro qual a operacao (comando escolhido pelo usuario) associada ele. */
    char nomeArquivo[DIMENSAO_GERAL];    /* Armazena o nome do arquivo sendo transferido, caso o pacote seja usado para transferencia de arquivos. */
    char usuario[DIMENSAO_NOME_USUARIO]; /* Armazena o usuario envolvido na operacao referente ao pacote, na comunicacao com o servidor. */

    uint32_t tamanho;      /* Indica a quantidade de bytes do pacote.*/
    int codigoComunicacao; /* Valor inteiro para indicar a operacao (comando escolhido pelo usuario) associado ao pacote, usado tambem para identificar erros de comunicacao. */

} Pacote;

#endif