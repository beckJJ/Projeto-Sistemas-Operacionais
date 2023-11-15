#ifndef _INTERFACECLIENTE_H_
#define _INTERFACECLIENTE_H_

#include "../Common/structs.h"

/* Constantes para indicar a sintaxe de cada comando. */
#define COMANDO_MYCLIENT "myClient"
#define COMANDO_UPLOAD "upload"
#define COMANDO_DOWNLOAD "download"
#define COMANDO_DELETE "delete"
#define COMANDO_LISTSERVER "list_server"
#define COMANDO_LISTCLIENT "list_client"
#define COMANDO_EXIT "exit"
#define COMANDO_HELP "help"

/* Constantes para indicar a quantidade de parametros de cada comando. */
#define QUANTIDADE_PARAMETROS_MYCLIENT 3
#define QUANTIDADE_PARAMETROS_UPLOAD 1
#define QUANTIDADE_PARAMETROS_DOWNLOAD 1
#define QUANTIDADE_PARAMETROS_DELETE 1

void menu_help();
void menu_principal(DadosConexao dados_conexao);
void executa_comando(DadosConexao dados_conexao);

#endif

