#ifndef _COMUNICACAOSERVIDOR_H_
#define _COMUNICACAOSERVIDOR_H_

#include "../Common/structs.h"
#include "../Common/comunicacao.h"
#include "config.h"

void download(int newsocket_id, Pacote *pacote);		/* Envia um pacote contendo um arquivo, que foi solicitado por um cliente. */
void listarArquivosDiretorio(char *diretorio, Pacote *pacote);	/* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente em um pacote. */
void list_server(int newsocket_id, Pacote *pacote);		/* Lista informacoes sobre os arquivos presentes no repositorio remoto do cliente em um pacote, e envia o pacote ao cliente solicitante. */
void upload(char *diretorioDestino, Pacote *pacote);		/* Armazena um arquivo enviado pelo cliente, no repositorio remoto associado a ele. */
void imprimeDadosPacote(Pacote pacote); 			/* Imprime, no terminal, os dados relacionados com um pacote que foi recebido pelo servidor ou enviado pelo servidor. */

#endif
