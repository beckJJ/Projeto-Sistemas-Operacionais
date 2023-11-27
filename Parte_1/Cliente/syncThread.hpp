#ifndef _SYNC_THREAD_H_
#define _SYNC_THREAD_H_

// Thread de sincronização responsável por receber PackageChangeEvent do servidor e alterar arquivos
//   locais correspondentemente

// Lê eventos enviados pelo servidor
void *syncThread(void *);

#endif
