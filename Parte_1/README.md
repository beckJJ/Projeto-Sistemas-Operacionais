# Projeto Sistemas Operacionais II - Parte 1

## Geral

As operações de leitura e escrita de pacotes deverão ser feitas pelas funções `read_package_from_socket` e `write_package_to_socket`. Essas funções irão exibir os pacotes recebidos e enviados no `stderr`, caso a macro `DEBUG_PACOTE` seja verdadeira.

Antes de serem enviados, o conteúdo dos pacotes é convertido para o formato big-endian. Ao serem recebidos por um destinatário (cliente ou servidor), o conteúdo é convertido para a representação local, utilizada no sistema do destinatário. Todos os campos dos pacotes são declarados com utilização da primitiva alignas(), dessa forma garantindo um padding bem definido e simples de calcular. 

## Conexão

A conexão foi desenvolvida com a expectativa de que ao iniciarmos o envio de uma sequência de pacotes, essa sequência continuará sendo enviada até o fim. Em suma, o envio de um arquivo impedirá o envio de eventos. Essa medida foi adotada para simplificar a comunicação entre cliente e servidor.

Para o cliente, há três threads (que representam todas as threads utilizadas na aplicação cliente) com acesso ao socket. Para impedir que ocorram múltiplas escritas ao mesmo tempo, usamos mutexes. Não é necessário sincronização na leitura, pois é a leitura é feita por apenas uma thread.

A thread de eventThread, responsável por escutar eventos do inotify do sync_dir local, acessa o socket para escrita. Em seguida, o evento FILE_MODIFIED será seguido do conteúdo modificado, e o envio é protegido por um mutex_lock. A thread de readThread, responsável por receber os pacotes enviados pelo servidor para o cliente, ficará lendo pacotes enviados do servidor. Esses pacotes podem ser pacotes de eventos ou pacotes associados às respostas de requisições feitas pelo usuário (há um `pthread_cond_t` associado aos pacotes de listagem dos arquivos). A thread em myClient, responsável por ler os comandos digitados pelo usuário, utiliza a mesma mutex_lock da thread eventThread, para enviar as requisições para o servidor.

No servidor existem as seguintes definições: Usuários, usuário e dispositivo. Usuários é um `std::map` associando o nome de usuário a uma estrutura de usuário, alterações em usuários é protegido por mutex_lock. Um usuário é uma estrutura que tem informações sobre os dispositivos e listagem dos arquivos em memória. Para alterar os dispositivos, usamos uma mutex lock. Para alterar os arquivos usamos outra mutex_lock. Um dispositivo é uma conexão específica de um usuário, e cada dispositivo possui uma `pthread_t`, sendo a thread atualmente empregada a comunicar-se com o usuário, também sendo realizada uma associação com o socket utilizado e uma mutex lock para protegê-la. A mutex lock é adquirida tanto para eventos quanto para resposta de requisições, geradas pela interface.

Para simplificar a comunicação, após um evento FILE_MODIFIED, o conteúdo do arquivo modificado é recebido logo em seguida.

### Comunicação dos pacotes

A comunicação é iniciada com a identificação do usuário, um usuário deve informar seu nome e o servidor irá responder com o ID do dispositivo no caso de sucesso, ou rejeitará a conexão no caso de já existirem dois dispositivos conectados.

As tabelas demonstram os pacotes enviados em ordem, \[alt. n\] representam alternativas.

Identificação (primeira etapa):

| Agente   | Pacote                             | Descrição                                                                                         |
| -------- | ---------------------------------- | ------------------------------------------------------------------------------------------------- |
| cliente  | PackageUserIndentification         | Identificação inicial do usuário, deverá conter seu nome e indicar tipo de conexão como principal |
| servidor | PackageUserIndentificationResponse | Indica se conexão foi aceita ou rejeitada, caso aceita deverá conter o ID do dispositivo          |

Loop de pacotes cliente -> servidor:

| Agente                             | Pacote                           | Descrição                                                                                                     |
| ---------------------------------- | -------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| \[alt. 1\] cliente   (interface)   | PackageRequestFileList           | Usuário deseja receber a lista de arquivos presentes no servidor                                              |
| \[alt. 1\] servidor                | PackageFileList                  | Item da lista de arquivo (campo de tamanho para quantos pacotes precisa ler, considera que já leu o primeiro) |
| \[alt. 2\] cliente   (eventThread) | PackageChangeEvent FILE_DELETED  | Usuário removeu o arquivo filename1 de seu diretório sync dir local                                           |
| \[alt. 3\] cliente   (eventThread) | PackageChangeEvent FILE_CREATED  | Usuário criou o arquivo filename1 em seu diretório sync dir local                                             |
| \[alt. 4\] cliente   (eventThread) | PackageChangeEvent FILE_MODIFIED | Usuário modificou o arquivo filename1 em seu diretório sync dir local                                         |
| \[alt. 4-1\] cliente (eventThread) | PackageFileNotFound              | Usuário iria enviar o arquivo modificado, mas não foi possível acessá-lo                                      |
| \[alt. 4-2\] cliente (eventThread) | PackageUploadFile                | Usuário enviará o conteúdo do arquivo modificado                                                              |
| \[alt. 4-2\] cliente (eventThread) | PackageFileContent               | Conteúdo do arquivo modificado                                                                                |
| \[alt. 5\] cliente   (eventThread) | PackageChangeEvent FILE_RENAME   | Usuário renomeou o arquivo filename1 para filename2 em seu diretório sync dir local                           |

Loop de pacotes servidor -> cliente:

| Agente                | Pacote                           | Descrição                                                                                           |
| --------------------- | -------------------------------- | --------------------------------------------------------------------------------------------------- |
| \[alt. 1\] servidor   | PackageFileList                  | Item da lista de arquivo (enviado apenas como resposta à requisição PackageRequestFileList prévia) |
| \[alt. 2\] servidor   | PackageChangeEvent FILE_DELETED  | Outro dispositivo removeu o arquivo filename1 de seu diretório sync dir local                       |
| \[alt. 3\] servidor   | PackageChangeEvent FILE_CREATED  | Outro dispositivo criou o arquivo filename1 em seu diretório sync dir local                         |
| \[alt. 4\] servidor   | PackageChangeEvent FILE_MODIFIED | Outro dispositivo modificou o arquivo filename1 em seu diretório sync dir local                     |
| \[alt. 4-1\] servidor | PackageFileNotFound              | O arquivo seria enviado, mas não foi possível acessá-lo                                             |
| \[alt. 4-2\] servidor | PackageUploadFile                | Conteúdo do arquivo modificado será enviado                                                         |
| \[alt. 4-2\] servidor | PackageFileContent               | Conteúdo do arquivo modificado                                                                      |
| \[alt. 5\] servidor   | PackageChangeEvent FILE_RENAME   | Outro dispositivo renomeou o arquivo filename1 para filename2 em seu diretório sync dir local       |

## Servidor

Cada usuário terá no máximo dois dispositivos.

Cada dispositivo tem um ID para diferenciá-los, eventos gerados por um dispositivo não serão reenviados para esse mesmo dispositivo, verifica-se o ID do mesmo.

A leitura e modificação da lista de arquivos em memória é protegida por uma mutex_lock (para cada usuário, dispositivos a compartilham). (ver TODO)

A leitura e modificação da lista de dispositivos é protegida por uma mutex_lock (para cada usuário, dispositivos a compartilham).

A leitura e modificação da map de usuários é protegida por uma mutex_lock.

## Cliente

O myClient utiliza três threads: principal, evento e leitura.

A thread principal espera por comando digitados pelo usuario.

A thread de eventos observa o diretório sync_dir local e envia os eventos gerados para o servidor.

A thread de leitura recebe pacotes do servidor, excepicionalmente o pacote PackageFileList envolve sinalização de condição, pois o comando list_server origina uma requisição para a listagem de arquivos no servidor. (*)

A escrita na socket é protegida por mutex.

A escrita na socket é feita por apenas um thread, não é usado mutex.

*: A condição que será sinalizada e aguardada é a condição de já ter recebido a listagem de arquivos, a thread que executa comandos do usuário enviará o pacote requisitando arquivos e então aguardará pelo sinal, com o sinal recebido a thread então exibirá a listagem dos arquivos. A thread de leitura deverá sinalizar que a leitura foi concluída quando terminar de ler os pacotes de listagem de arquivos.

### get_sync_dir

Para implementar o get_sync_dir o diretório local é recursivamente removido, então é requisitado a lista de arquivos presentes no servidor, todos os arquivos são baixados para o diretório recém limpo. Executado antes da inicialização da thread do inotifiy.

### upload

O arquivo do usuário é copiado para o diretório local, o envio é feito pela thread de eventos que receberá os eventos de criação de modificação do inotify.

### download

Arquivo presente no diretório sync_dir local é copiado para o cwd.

### delete

O arquivo é removido do diretório sync_dir local, o evento é enviado pela thread de eventos que receberá um evento do inotify.

### list_client

Lista arquivos do sync dir local.

### list_server

Recebe listagem dos arquivos no servidor e os exibe.

## Sincronização

Para sincronização foram usadas mutexes, leituras são mutuamente exclusivas, há possibilidade de aperfeiçoamento.

É usado cond_signal e cond_wait para leitura da listagem de arquivos no usuário.

## Problemas encontrados

### Loop de notificações

Nos casos em que existiam dois dispositivos de um mesmo usuário conectados ao servidor, havia a possibilidade de ocorrer um loop de notificações, da forma exemplificada abaixo. 

```text
[id=0x01] Dispositivo atual onde o arquivo 'teste' foi renomeado para 'exemplo':
1)  WRITE Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- Gerado pelo inotify, será propagado
4)  READ  Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- Evento recebido pela propagação de 3)
5)  Erro ao renomear arquivo de "teste" para "exemplo".             <- Erro, o arquivo 'teste' não existe mais

[id=0x02] Segundo Dispositivo:
2)  READ  Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- Evento recebido pela propagação de 1)
3)  WRITE Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- Gerado pelo inotify, será propagado
```

Para garantir que isso seja evitado, o último evento de notificação é armazenado no usuário. Antes do usuário enviar o evento para o servidor, verifica-se se o evento em questão é igual ao que já foi lido anteriormente. Caso seja, o evento é ignorado. O código referente à essa verificação encontra-se em Cliente/eventThread.cpp:188.

```cpp
// Envia eventos completados
for (auto userEvent : completeUserEvents)
{
    // ...

    // Ignora eventos gerados pelo inotify que foram gerados em resposta ao último evento
    //   de alteração enviado pelo servidor:
    //   READ  Package(CHANGE_EVENT, 0x01, FILE_RENAME, teste, exemplo)  <- previousEvent
    //   WRITE Package(CHANGE_EVENT, 0x02, FILE_RENAME, teste, exemplo)  <- changeEvent
    if ((previousEvent.event == changeEvent.event) &&
        (!strcmp(previousEvent.filename1, changeEvent.filename1)) &&
        (!strcmp(previousEvent.filename2, changeEvent.filename2)))
    {
        // Limpa previousSyncedChangeEvent, se não fosse limpo eventos genuínos seriam
        //   ignorados
        pthread_mutex_lock(&previousSyncedChangeEventLock);
        previousSyncedChangeEvent = PackageChangeEvent((ChangeEvents)0xff, (uint8_t)0xff, "", "");
        pthread_mutex_unlock(&previousSyncedChangeEventLock);
        continue;
    }

    // ...
}
```

### IN_CLOSE_WRITE após IN_CREATE

Para a criação de arquivos (ou seja, nos casos em que é recebido um evento FILE_CREATE do servidor), utiliza-se fopen(file, "w"). A criação do arquivo file irá gerar dois eventos em seguida: IN_CREATE e IN_CLOSE_WRITE. O evento IN_CLOSE_WRITE enviará, então, o conteúdo atualizado para o servidor, que o salvará no lugar correto. O problema ocorre quando são utilizados dois dispositivos A e B por um mesmo usuário e, no dispositivo A, um arquivo qualquer é copiado para o diretório sincronizado. Nesse caso, normalmente o inotify enviaria os eventos corretamente, e o arquivo seria salvo no servidor. Porém, para aplicar os eventos criados, o dispositivo B criaria o arquivo file, o que geraria os eventos IN_CREATE e IN_CLOSE_WRITE.  Assim, temos um segundo IN_CLOSE_WRITE. Com isso, o servidor receberia o conteúdo vazio do arquivo recém-criado, e substituiria o conteúdo que havia sido armazenado do dispositivo A. Como resultado, qualquer arquivo adicionado não teria conteúdo algum. 

Para solucionar esse problema, é feita uma verificação para analisar (antes de realizar o envio da notificação do evento FILE_MODIFIED para o servidor) se o evento anterior correspondia a um IN_CREATE do mesmo arquivo, e se o arquivo modificado está vazio. Essas verificações indicam que o evento IN_CLOSE_WRITE foi gerado pela chamada de fopen(file, "w"). Nesse caso o evento não será enviado para o servidor. A verificação se encontra em Client/eventThread.cpp:164. 

```cpp
// Adiciona FILE_MODIFIED caso IN_CLOSE_WRITE não seja disparado por criação de arquivo
// Ver README
if (
    !(
        // Evento anterior foi criação do arquivo modificado
        (previousEvent.changeEvent == FILE_CREATED && previousEvent.movedFrom == filename) &&
        // Arquivo modificado não tem conteúdo (fopen(file, "w"))
        ((stat(path.c_str(), &st) == 0) && st.st_size == 0)))
{
    completeUserEvents.push_back({FILE_MODIFIED, filename, std::string(""), 0});
    previousEvent = completeUserEvents.back();
}
```
