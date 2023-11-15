#ifndef _COMUNICACAOCLIENTE_H_
#define _COMUNICACAOCLIENTE_H_

#include "../Common/structs.h"
#include "../Common/comunicacao.h"

int conecta_servidor(DadosConexao dados_conexao); /* Inicia a conexao com o servidor, via socket. */
void upload(DadosConexao dados_conexao); /* Realiza o envio de um arquivo para o diretorio do usuario, presente no servidor. */
void download (DadosConexao dados_conexao); /* Recebe um arquivo do servidor, e copia para o diretorio de onde a aplicacao cliente foi chamada. */
void delete(DadosConexao dados_conexao); /* Deleta um arquivo presente na maquina local do usuario, no diretorio sync_dir_<usuario> */
void list_server(DadosConexao dados_conexao); /* Lista os arquivos armazenados no repositorio remoto do servidor, associados ao usuario. */
void list_client(DadosConexao dados_conexao); /* Lista todos os arquivos do repositorio local syn_dir_<usuario>, junto com os MAC times. */

#endif
