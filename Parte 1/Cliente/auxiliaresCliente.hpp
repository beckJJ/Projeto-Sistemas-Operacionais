#ifndef _AUXILIARESCLIENTE_H_
#define _AUXILIARESCLIENTE_H_

#include "../Common/structs.hpp"

void inicializaPacote(Pacote *pacote); /* Inicializa o pacote que ira conter os dados da interacao com o servidor. */
int verificaParametros(char *comando, int quantidade_parametros); /* Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido. */
void analisa_diretorio(char *nome_usuario); /* Cria ou carrega o diretorio sync_dir_<usuario> */
void listarArquivosDiretorio(char *diretorio); /* Lista todos os arquivos de um determinado diretorio, junto com os MAC times. */
void obtemNomeArquivoComando(char *comando, char *nomeArquivo); /* Recebe um comando digitado pelo usuario, e obtem o nome do arquivo final referenciado no comando. */
void obtemDiretorioComando(char *comando, char *diretorio); /* Recebe um comando digitado pelo usuario, e obtem o nome do diretorio referenciado no comando. */
void obtemNomeArquivoDiretorio(char *caminhoCompleto, char *nomeArquivo); /* Recebe um diretorio escolhido pelo usuario, e obtem o nome do arquivo final referenciado no diretorio. */

#endif
