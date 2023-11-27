# Projeto Sistemas Operacionais II - Parte 1

## TODO

O pacote do tipo UPLOAD_FILE é enviado pela conexão principal do usuário para o servidor?

syncThread e eventThread do cliente escrevem na mesma socket de eventos, para remover a necessidade de mutex uma idéia seria ter um vetor de requisições a serem feitas pela socket principal.

Permitir múltiplas leituras dos arquivos do usuário no servidor.

Corrigir alteração de mesmo arquivo em dois dispositivos sendo suprimida.

## Geral

Toda escrita e leitura base de pacotes deverão ser feitas pelas funções são feitas pelas funções `read_package_from_socket` e `write_package_to_socket`. Essas funções irão exibir os pacotes recebidos e enviados no `stderr` caso a macro `DEBUG_PACOTE` sejá verdadeira.

Os pacotes antes de serem enviados são convertido para big-endian, na leitura esses são convertidos para a representação local. As duas funções acima executam as conversões automaticamente. Os pacotes são declarados com todos os seus campos com alingas(8) para ter um padding bem definido e simples de calcular.

## Conexão

Há dois tipos de conexão para cada dispositivo: conexão principal e conexão de eventos. A conexão principal espera pelos pacotes que são enviados pelos comandos que o usuário digita. A conexão de eventos é onde o servidor espera por pacotes do tipo CHANGE_EVENT, indicando modificações nos arquivos, no caso de um evento FILE_MODIFIED o conteúdo é enviado em seguida.

As tabelas demonstram a conexão em ordem, \[alt. nn\] representam alternativas.

### Conexão principal

Identificação:

Agente|Pacote|Descrição
-|-|-
cliente|PackageUserIndentification|Identificação inicial do usuário, deverá conter seu nome e indicar tipo de conexão como principal
servidor|PackageUserIndentificationResponse|Indica se conexão foi aceita ou rejeitada, caso aceita deverá conter o ID do dispositivo

Loop de pacotes cliente -> servidor:

Agente|Pacote|Descrição
-|-|-
\[alt. 1\] cliente|PackageRequestFile|Usuário deseja receber um arquivo do servidor
\[alt. 1-1\] servidor|PackageFileNotFound|Arquivo não encontrado na lista de arquivos do usuário
\[alt. 1-2\] servidor|PackageUploadFile|Indica que será feito upload do arquivo arquivo indicado pelo pacote
\[alt. 1-2\] servidor|PackageFileContent|Conteúdo do arquivo requisitado
\[alt. 2\] cliente|PackageRequestFileList|Usuário deseja receber a lista de arquivos presentes no servidor
\[alt. 2\] servidor|PackageFileList|Item da lista de arquivo (campo de tamanho para quantos pacotes precisa ler, considera que já leu o primeiro)
\[alt. 3\] cliente|PackageUploadFile|Usuário vai enviar um arquivo que deve ser salvo
\[alt. 3\] cliente|PackageFileContent|Conteúdo de um arquivo (previamente definido sobre qual se trata)

Loop de pacotes servidor -> client:

Não há

### Conexão de eventos

Identificação:

Agente|Pacote|Descrição
-|-|-
cliente|PackageUserIndentification|Identificação inicial do usuário, deverá conter seu nome, ID do dispositivo e indicar tipo de conexão como de evento
servidor|PackageUserIndentificationResponse|Indica se conexão foi aceita ou rejeitada, caso aceita deverá conter o ID do dispositivo

Loop de pacotes cliente -> servidor:

Agente|Pacote|Descrição
-|-|-
\[alt. 1\] cliente|PackageChangeEvent FILE_DELETED|Usuário removeu o arquivo filename1 de seu diretório sync dir local
\[alt. 2\] cliente|PackageChangeEvent FILE_CREATED|Usuário criou o arquivo filename1 em seu diretório sync dir local
\[alt. 3\] cliente|PackageChangeEvent FILE_MODIFIED|Usuário modificou o arquivo filename1 em seu diretório sync dir local
\[alt. 3-1\] cliente|PackageFileNotFound|Usuário iria enviar o arquivo modificado, mas não foi possível acessá-lo
\[alt. 3-2\] cliente|PackageUploadFile|Usuário enviará o conteúdo do arquivo modificado
\[alt. 3-2\] cliente|PackageFileContent|Conteúdo do arquivo modificado
\[alt. 4\] cliente|PackageChangeEvent FILE_RENAME|Usuário renomeou o arquivo filename1 para filename2 em seu diretório sync dir local

Loop de pacotes servidor -> client:

Agente|Pacote|Descrição
-|-|-
\[alt. 1\] servidor|PackageChangeEvent FILE_DELETED|Outro dispositivo removeu o arquivo filename1 de seu diretório sync dir local
\[alt. 2\] servidor|PackageChangeEvent FILE_CREATED|Outro dispositivo criou o arquivo filename1 em seu diretório sync dir local
\[alt. 3\] servidor|PackageChangeEvent FILE_MODIFIED|Outro dispositivo modificou o arquivo filename1 em seu diretório sync dir local
\[alt. 3\] cliente|PackageRequestFile|Pede para que o servidor envie o arquivo modificado
\[alt. 3-1\] servidor|PackageFileNotFound|O arquivo seria enviado, mas não foi possível acessá-lo
\[alt. 3-2\] servidor|PackageUploadFile|Conteúdo do arquivo modificado será enviado
\[alt. 3-2\] servidor|PackageFileContent|Conteúdo do arquivo modificado
\[alt. 4\] servidor|PackageChangeEvent FILE_RENAME|Outro dispositivo renomeou o arquivo filename1 para filename2 em seu diretório sync dir local

## Servidor

Cada usuário terá no máximo dois dispositivos.

Cada dispositivo tem um ID para diferenciá-los, eventos gerados por um dispositivo não serão reenviados para esse mesmo dispositivo, verifica-se o ID do mesmo.

A leitura e modificação da lista de arquivos em memória é protegida por uma mutex_lock (para cada usuário, dispositivos a compartilham). (ver TODO)

A leitura e modificação da lista de dispositivos é protegida por uma mutex_lock (para cada usuário, dispositivos a compartilham).

A leitura e modificação da map de usuários é protegida por uma mutex_lock.

## Cliente

O myClient utiliza três threads: principal, evento e sincronização.

A thread principal espera por comando digitados pelo usuario.

A thread de eventos observa o diretório sync_dir local e envia os eventos gerados para o servidor.

A thread de sincronização escuta pelos eventos enviados do servidor para o cliente indicando que determinado arquivo teve alguma alteração.

A thread principal utiliza a socket principal, já a thread de eventos e sincronização compartilham a socket de eventos.

A socket de eventos é protegida por um lock mutualmente exclusiva. (ver TODO)

### get_sync_dir

Para implementar o get_sync_dir o diretório local é recursivamente removido, então é requisitado a lista de arquivos presentes no servidor, todos os arquivos são baixados para o diretório recém limpo.

### upload

O arquivo do usuário é copiado para o diretório local, o envio é feito pela thread de eventos que receberá os eventos de criação de modificação do inotify.

### download

É feita uma requisição para obter o arquivo, caso bem-sucedido o arquivo será salvo.

### delete

O arquivo é removido do diretório sync_dir local, o evento é enviado pela thread de eventos que receberá um evento do inotify.

### list_client

Lista arquivos do sync dir local.

### delete

Recebe listagem dos arquivos no servidor e os exibe.

## Sincronização

Para sincronização foram usadas mutexes, leituras são mutuamente exclusivas, há possibilidade de aperfeiçoamento.

## Problemas encontrados

Com dois dispositivos era possível obter um loop de notificações de eventos:

```text
[id=0x01] Dispositivo atual onde o arquivo 'teste' foi renomeado para 'exemplo':
1)  WRITE Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- Gerado pelo inotify, será propagado
4)  READ  Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- Evento recebido pela propagação de 3)
5)  Erro ao renomear arquivo de "teste" para "exemplo".             <- Erro, o arquivo 'teste' não existe mais

[id=0x02] Segundo Dispositivo:
2)  READ  Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- Evento recebido pela propagação de 1)
3)  WRITE Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- Gerado pelo inotify, será propagado
```

Para evitar isso o último evento de notificação enviado pelo servidor é armazenado no usuário, antes do usuário enviar o evento para o servidor é verificado se o evento é igual ao lido anteriormente (com exceção do id do dispositivo), se for o evento é ignorado. A verificação se encontra em Client/eventThread.cpp:164.

```cpp
// Envia eventos completados
for (auto userEvent : completeUserEvents)
{
    // ...

    // Ignora eventos gerados pelo inotify que foram gerados em resposta ao último evento
    //   de alteração enviado pelo servidor:
    //   READ  Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- previousEvent
    //   WRITE Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- changeEvent
    if ((previousEvent.deviceID != changeEvent.deviceID) &&
        (previousEvent.event == changeEvent.event) &&
        (!strcmp(previousEvent.filename1, changeEvent.filename1)) &&
        (!strcmp(previousEvent.filename2, changeEvent.filename2)))
    {
        continue;
    }

    // ...
}
```

Porém, com isso perde-se a possibilidade de se alterar o mesmo arquivo em dois dispositivo, se o último evento foi uma modificação do x pelo dispositivo 1, alterações feitas pelo dispositivo 2 serão ignoradas.
