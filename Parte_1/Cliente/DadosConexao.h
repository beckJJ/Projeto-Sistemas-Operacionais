#ifndef _DADO_CONEXAO_H_
#define _DADO_CONEXAO_H_

#define DIMENSAO_GERAL 100
#define DIMENSAO_COMANDO 200
#define DIMENSAO_NOME_USUARIO 50

/* Estrutura utilizada para conter todos os dados da conexao. */
typedef struct
{
    char nome_usuario[DIMENSAO_NOME_USUARIO]; /* Armazena o nome do usuario que estabeleceu conexao com o servidor. */
    char endereco_ip[DIMENSAO_GERAL];         /* Armazena o endereco IP do servidor. */
    char numero_porta[DIMENSAO_GERAL];        /* Armazena o numero da porta, usado para estabelecer conexao cliente-servidor. */

    char comando[DIMENSAO_COMANDO]; /* Armazena o comando da vez, escolhido pelo usuario. */

    int socket_id; /* Armazena o valor inteiro que identifica o socket sendo utilizado para comunicacao. */
} DadosConexao;

#endif
