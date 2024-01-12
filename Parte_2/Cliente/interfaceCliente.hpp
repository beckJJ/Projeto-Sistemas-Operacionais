#ifndef _INTERFACECLIENTE_H_
#define _INTERFACECLIENTE_H_

#include "DadosConexao.hpp"

/* Constantes para indicar a sintaxe de cada comando. */
#define COMANDO_MYCLIENT "myClient"
#define COMANDO_UPLOAD "upload"
#define COMANDO_DOWNLOAD "download"
#define COMANDO_DELETE "delete"
#define COMANDO_LISTSERVER "list_server"
#define COMANDO_LISTCLIENT "list_client"
#define COMANDO_EXIT "exit"
#define COMANDO_HELP "help"

#define ID_COMANDO_UPLOAD 1
#define ID_COMANDO_DOWNLOAD 2
#define ID_COMANDO_DELETE 3
#define ID_COMANDO_LISTSERVER 4
#define ID_COMANDO_LISTCLIENT 5
#define ID_COMANDO_EXIT 6
#define ID_COMANDO_HELP 7

/* Constantes para indicar a quantidade de parametros de cada comando. */
#define QUANTIDADE_PARAMETROS_MYCLIENT 3
#define QUANTIDADE_PARAMETROS_UPLOAD 1
#define QUANTIDADE_PARAMETROS_DOWNLOAD 1
#define QUANTIDADE_PARAMETROS_DELETE 1

void menu_help_ocmandos();
void menu_help(int comando_id);
void menu_principal(DadosConexao &dados_conexao);
// 1 = comando exit executado
int executa_comando(DadosConexao &dados_conexao);
int limpaTela();

#endif
