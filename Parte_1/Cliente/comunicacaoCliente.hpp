#ifndef _COMUNICACAOCLIENTE_H_
#define _COMUNICACAOCLIENTE_H_

// Funções de comunicação usadas pelo usuário

#include "DadosConexao.hpp"
#include <optional>

// Conecta-se com o servidor
std::optional<int> conecta_servidor(DadosConexao &dados_conexao);

// Copia o arquivo informado pelo usuário para o diretório sync_dir local, thread de eventos que
//   observa eventos do inotify deverá gerar os eventos apropriados
void upload(DadosConexao &dados_conexao);

// Pede arquivo do servidor, caso esteja disponível, será salvo no cwd atual
void download(DadosConexao &dados_conexao);

// Deleta um arquivo em sync_dir
void delete_cmd(DadosConexao &dados_conexao);

// Pede lista de arquivos para o servidor e exibe a lista após a condição
//   dados_conexao.is_file_list_readed ter sido completa pela thread de leitura
void list_server(DadosConexao &dados_conexao);

// Lista arquivos presentes no sync_dir local
void list_client(DadosConexao &dados_conexao);

#endif
