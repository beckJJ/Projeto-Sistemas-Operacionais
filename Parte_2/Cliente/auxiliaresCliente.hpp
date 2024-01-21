#ifndef _AUXILIARESCLIENTE_H_
#define _AUXILIARESCLIENTE_H_

#include <vector>
#include "../Common/package.hpp"
#include "../Common/DadosConexao.hpp"

// download_file pode falhar pelo arquivo não existir ou não conseguir ler ou escrever pacote
#define DOWNLOAD_FILE_FILE_NOT_FOUND 1
#define DOWNLOAD_FILE_ERROR 2

// Macros usadas para configurar exibiçao de File em print_files
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define TIME_FORMATTED_BUFFER 80
#define HEADER_FORMAT "%-30s %-7s %-20s %-20s %-20s\n"
#define FILE_FORMAT "%-30.30s %7ld %-20s %-20s %-20s\n"

// Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido.
int verificaParametros(char *comando, int quantidade_parametros);

// Separa campo comando de dados_conexao em um vetor de char * apontando para o início de palavra
//   contida no comando
std::vector<char *> splitComando(char *comando);

// Imprime uma lista de File's
void print_files(std::vector<File> files);

// Faz requisição para conectar dispositivo, dadosConexao terá campo correspondente alterado
int conecta_device(DadosConexao &dadosConexao);

// Remove sync_dir_user local e baixa os arquivos presentes no servidor
// Requisitará a lista de arquivos do servidor, então para cada arquivo dessa lista o arquivo será
//   requisitado e salvo em sync_dir
int get_sync_dir(DadosConexao &dadosConexao);

// Remove path e seus filhos recursivamente
int remove_path_tree(const char *path);

// Envia requisição para baixar arquivo e o salva em out_path
int download_file(int socket_id, const char *filename, const char *out_path);

#endif
