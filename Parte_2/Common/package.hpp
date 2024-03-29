#pragma once
#ifndef _PACKAGE_H_
#define _PACKAGE_H_

// Define os pacotes usados

#include <cstdint>
#include <vector>
#include <climits>

// Define alinhamento usado pelas estruturas e seus campos, facilita a determinação de quanto
//   padding haverá entre campos
#define ALIGN_VALUE 8

// Tamanho máximo que será enviado no buffer de conteúdo do pacote FILE_CONTENT
#define MAX_DATA_SIZE 4096

// Tamanho máximo do nome de usuário
#define USER_NAME_MAX_LENGTH 128

// Tipo de pacotes
enum PackageType : char
{
    // Usado para que o usuário se identifique
    INITIAL_USER_IDENTIFICATION,
    // Resposta do servidor para identificação do usuario
    USER_IDENTIFICATION_RESPONSE,
    // Indica mudança nos arquivos, tanto cliente->servidor quanto servidor->cliente
    CHANGE_EVENT,
    // Requisita um arquivo
    REQUEST_FILE,
    // Pacote contendo conteúdo de um arquivo
    FILE_CONTENT,
    // Requisita listagem de arquivos
    REQUEST_FILE_LIST,
    // Um item da lista de arquivos
    FILE_LIST,
    // Indica que um arquivo será enviado
    UPLOAD_FILE,
    // Indica que um arquivo que seria enviado não foi encontrado
    FILE_NOT_FOUND,
    // Usado para que um novo RM se identifique
    INITIAL_REPLICA_MANAGER_IDENTIFICATION,
    // Resposta do servidor para identificação do replica manager
    REPLICA_MANAGER_IDENTIFICATION_RESPONSE,
    // Pacote de ping enviado pelo backup
    REPLICA_MANAGER_PING,
    // Resposta do servidor para o ping do backup
    REPLICA_MANAGER_PING_RESPONSE,
    // Pacote de identificação da thread de transferência de arquivos do backup para o servidor principal
    REPLICA_MANAGER_TRANSFER_IDENTIFICATION,
    // Pacote de resposta do servidor principal para a thread de transferência de arquivos do servidor principal para o backup
    REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE,
    // Pacote para transferencia de lista de dispositivos conectados
    ACTIVE_CONNECTIONS_LIST,
    // Pacote para inicio de eleição (pacote election)
    REPLICA_MANAGER_ELECTION_ELECTION,
    // Pacote para resposta de eleição (pacote answer)
    REPLICA_MANAGER_ELECTION_ANSWER,
    // Pacote para coordenador de eleição (pacote coordinator)
    REPLICA_MANAGER_ELECTION_COORDINATOR,
    // Pacote para enviar informações do novo servidor para os clientes
    NEW_SERVER_INFO
};

// Indica aceitação ou rejeição da conexão do dispositivo
enum InitialUserIdentificationResponseStatus : char
{
    ACCEPTED,
    REJECTED,
};

// Indica aceitação ou rejeição da conexão do Replica Manager
enum InitialReplicaManagerIdentificationResponseStatus : char
{
    ACCEPTED_RM,
    REJECTED_RM,
};

enum ReplicaManagerTransferIdentificationResponseStatus : char
{
    ACCEPTED_RM_T,
    REJECTED_RM_T,
};

// Modificações que ocorreram em determinado arquivo, enviado pelo cliente com base no inotify e
//   propagado pelo servidor para outros dispositivos.
enum ChangeEvents : char
{
    FILE_DELETED,
    FILE_CREATED,
    FILE_MODIFIED,
    FILE_RENAME,
};

// Informações de um arquivo
struct alignas(ALIGN_VALUE) File
{
    // Tamanho do arquivo
    alignas(ALIGN_VALUE) int64_t size = 0;
    // MAC-time
    alignas(ALIGN_VALUE) int64_t mtime = 0;
    alignas(ALIGN_VALUE) int64_t atime = 0;
    alignas(ALIGN_VALUE) int64_t ctime = 0;
    // Nome do arquivo
    alignas(ALIGN_VALUE) char name[NAME_MAX]{};

    File();
    File(int64_t size, int64_t mtime, int64_t atime, int64_t ctime, const char _name[NAME_MAX]);

    bool operator<(const File &a) const;
};

// Usado para que o usuário se identifique
struct alignas(ALIGN_VALUE) PackageUserIdentification
{
    // ID do dispositivo, usado caso connectionType == EVENT_CONNECTION
    alignas(ALIGN_VALUE) uint8_t deviceID;
    // Numero da porta que o frontend ficará aguardando novas conexões de backups eleitos
    alignas(ALIGN_VALUE) uint16_t listen_port;
    // Nome do usuário que deseja se conectar
    alignas(ALIGN_VALUE) char user_name[USER_NAME_MAX_LENGTH]{};

    PackageUserIdentification(uint8_t deviceID, uint16_t listen_port, const char _user_name[USER_NAME_MAX_LENGTH]);
};

// Resposta do servidor para identificação do usuário
struct alignas(ALIGN_VALUE) PackageUserIdentificationResponse
{
    // Indica se foi aceita ou rejeitada a conexão
    alignas(ALIGN_VALUE) InitialUserIdentificationResponseStatus status;
    // ID do dispositivo
    alignas(ALIGN_VALUE) uint8_t deviceID;

    PackageUserIdentificationResponse(InitialUserIdentificationResponseStatus status, uint8_t deviceID);
};

// Usado para que o Replica Manager se identifique
struct alignas(ALIGN_VALUE) PackageReplicaManagerIdentification
{
    // ID do RM, usado caso connectionType == EVENT_CONNECTION
    alignas(ALIGN_VALUE) uint8_t deviceID;

    PackageReplicaManagerIdentification(uint8_t deviceID);
};

// Resposta do servidor para identificação do Replica Manager
struct alignas(ALIGN_VALUE) PackageReplicaManagerIdentificationResponse
{
    // Indica se foi aceita ou rejeitada a conexão
    alignas(ALIGN_VALUE) InitialReplicaManagerIdentificationResponseStatus status;
    // ID do dispositivo
    alignas(ALIGN_VALUE) uint8_t deviceID;

    PackageReplicaManagerIdentificationResponse(InitialReplicaManagerIdentificationResponseStatus status, uint8_t deviceID);
};

// Usado para que o Replica Manager se identifique na thread de transfer
struct alignas(ALIGN_VALUE) PackageReplicaManagerTransferIdentification
{
    alignas(ALIGN_VALUE) uint8_t deviceID;
    alignas(ALIGN_VALUE) uint16_t listen_port;

    PackageReplicaManagerTransferIdentification(uint8_t deviceID, uint16_t listen_port);
};

// Resposta do servidor para identificação do replica manager de transferência
struct alignas(ALIGN_VALUE) PackageReplicaManagerTransferIdentificationResponse
{
    alignas(ALIGN_VALUE) ReplicaManagerTransferIdentificationResponseStatus status;
    alignas(ALIGN_VALUE) uint8_t deviceID;
    PackageReplicaManagerTransferIdentificationResponse(ReplicaManagerTransferIdentificationResponseStatus status, uint8_t deviceID);
};

// Indica mudança nos arquivos
struct alignas(ALIGN_VALUE) PackageChangeEvent
{
    // Evento
    alignas(ALIGN_VALUE) ChangeEvents event;
    // Dispositivo em que o evento foi gerado
    alignas(ALIGN_VALUE) uint8_t deviceID;
    // Nome de arquivo 1
    alignas(ALIGN_VALUE) char filename1[NAME_MAX]{};
    // Nome de arquivo 2, usado apenas no evento de renomeação
    alignas(ALIGN_VALUE) char filename2[NAME_MAX]{};
    // nome de usuario, usado para os backups saberem onde salvar
    alignas(ALIGN_VALUE) char  username[NAME_MAX]{};

    PackageChangeEvent();
    PackageChangeEvent(ChangeEvents event, uint8_t deviceID, const char _filename1[NAME_MAX], const char _filename2[NAME_MAX], const char _username[NAME_MAX]);
};

// Requisita um arquivo
struct alignas(ALIGN_VALUE) PackageRequestFile
{
    // Arquivo requisitado
    alignas(ALIGN_VALUE) char filename[NAME_MAX]{};

    PackageRequestFile(const char _filename[NAME_MAX]);
};

// Pacote contendo conteúdo de um arquivo
struct alignas(ALIGN_VALUE) PackageFileContent
{
    // Tamanho do arquivo sendo enviado
    alignas(ALIGN_VALUE) int64_t size;
    // Sequência do pacote de conteúdo sendo enviado, inicia em 1
    alignas(ALIGN_VALUE) uint16_t seqn;
    // Tamanho da sequência de bytes correspondente ao conteúdo do arquivo que virá depois desse
    //   pacote
    alignas(ALIGN_VALUE) uint16_t data_length;

    PackageFileContent(int64_t size, uint16_t seqn, uint16_t data_length);
};

// Requisita listagem de arquivos
struct alignas(ALIGN_VALUE) PackageRequestFileList
{
    PackageRequestFileList();
};

// Um item da lista de arquivos
struct alignas(ALIGN_VALUE) PackageFileList
{
    // Tamanho da lista sendo enviada
    alignas(ALIGN_VALUE) uint16_t count;
    // Sequência do item sendo enviado, inicia em 1
    alignas(ALIGN_VALUE) uint16_t seqn;
    // Informações sobre o arquivo atual
    alignas(ALIGN_VALUE) File file;

    PackageFileList(uint16_t count, uint16_t seqn, File file);
};

struct alignas(ALIGN_VALUE) Connection_t
{
    // Tipos usados em struct sockaddr_in
    alignas(ALIGN_VALUE) uint16_t port;
    alignas(ALIGN_VALUE) uint32_t host;
    alignas(ALIGN_VALUE) uint16_t socket_id;
    alignas(ALIGN_VALUE) char user_name[USER_NAME_MAX_LENGTH];

    Connection_t();
    Connection_t(uint16_t port, uint32_t host, uint16_t socket_id, const char _user_name[USER_NAME_MAX_LENGTH]);
};

struct alignas(ALIGN_VALUE) PackageActiveConnectionsList
{
    // Tamanho da lista sendo enviada
    alignas(ALIGN_VALUE) uint16_t count;
    // Sequência do item sendo enviado, inicia em 1
    alignas(ALIGN_VALUE) uint16_t seqn;
    // Indica se é cliente ou backup
    alignas(ALIGN_VALUE) bool is_client;
    // Conexão atual
    alignas(ALIGN_VALUE) Connection_t connection;

    PackageActiveConnectionsList(uint16_t count, uint16_t seqn, bool is_client, Connection_t connection);
};

// Indica que um arquivo será enviado
struct alignas(ALIGN_VALUE) PackageUploadFile : File
{
    PackageUploadFile(File file);
};

// Indica que um arquivo que seria enviado não foi encontrado
struct alignas(ALIGN_VALUE) PackageFileNotFound
{
    PackageFileNotFound();
};

struct alignas(ALIGN_VALUE) PackageReplicaManagerPing
{
    PackageReplicaManagerPing();
};

struct alignas(ALIGN_VALUE) PackageReplicaManagerPingResponse
{
    PackageReplicaManagerPingResponse();
};

struct alignas(ALIGN_VALUE) PackageReplicaManagerElectionElection
{
    PackageReplicaManagerElectionElection();
};

struct alignas(ALIGN_VALUE) PackageReplicaManagerElectionAnswer
{
    PackageReplicaManagerElectionAnswer();
};

struct alignas(ALIGN_VALUE) PackageReplicaManagerElectionCoordinator
{
    alignas(ALIGN_VALUE) uint8_t deviceID;

    PackageReplicaManagerElectionCoordinator(uint8_t deviceID);
};

struct alignas(ALIGN_VALUE) PackageNewServerInfo
{
    alignas(ALIGN_VALUE) uint16_t port;
    alignas(ALIGN_VALUE) uint32_t host;

    PackageNewServerInfo(uint16_t port, uint32_t host);
};


// União de todos os pacotes
union alignas(ALIGN_VALUE) PackageSpecific
{
    alignas(ALIGN_VALUE) PackageUserIdentification userIdentification;
    alignas(ALIGN_VALUE) PackageUserIdentificationResponse userIdentificationResponse;
    alignas(ALIGN_VALUE) PackageReplicaManagerIdentification replicaManagerIdentification;
    alignas(ALIGN_VALUE) PackageReplicaManagerIdentificationResponse replicaManagerIdentificationResponse;
    alignas(ALIGN_VALUE) PackageReplicaManagerTransferIdentification replicaManagerTransferIdentification;
    alignas(ALIGN_VALUE) PackageReplicaManagerTransferIdentificationResponse replicaManagerTransferIdentificationResponse;
    alignas(ALIGN_VALUE) PackageChangeEvent changeEvent;
    alignas(ALIGN_VALUE) PackageRequestFile requestFile;
    alignas(ALIGN_VALUE) PackageFileContent fileContent;
    alignas(ALIGN_VALUE) PackageFileList fileList;
    alignas(ALIGN_VALUE) PackageActiveConnectionsList activeConnectionsList;
    alignas(ALIGN_VALUE) PackageRequestFileList requestFileList;
    alignas(ALIGN_VALUE) PackageUploadFile uploadFile;
    alignas(ALIGN_VALUE) PackageFileNotFound fileNotFound;
    alignas(ALIGN_VALUE) PackageReplicaManagerPing replicaManagerPing;
    alignas(ALIGN_VALUE) PackageReplicaManagerPingResponse replicaManagerPingResponse;
    alignas(ALIGN_VALUE) PackageReplicaManagerElectionElection replicaManagerElectionElection;
    alignas(ALIGN_VALUE) PackageReplicaManagerElectionAnswer replicaManagerElectionAnswer;
    alignas(ALIGN_VALUE) PackageReplicaManagerElectionCoordinator replicaManagerElectionCoordinator;
    alignas(ALIGN_VALUE) PackageNewServerInfo newServerInfo;

    PackageSpecific();
    PackageSpecific(PackageUserIdentification userIdentification);
    PackageSpecific(PackageUserIdentificationResponse userIdentificationResponse);
    PackageSpecific(PackageReplicaManagerIdentification replicaManagerIdentification);
    PackageSpecific(PackageReplicaManagerIdentificationResponse replicaManagerIdentificationResponse);
    PackageSpecific(PackageReplicaManagerTransferIdentification replicaManagerTransferIdentification);
    PackageSpecific(PackageReplicaManagerTransferIdentificationResponse replicaManagerTransferIdentificationResponse);
    PackageSpecific(PackageChangeEvent ChangeEvent);
    PackageSpecific(PackageRequestFile requestFile);
    PackageSpecific(PackageFileContent fileContent);
    PackageSpecific(PackageRequestFileList requestFileList);
    PackageSpecific(PackageFileList fileList);
    PackageSpecific(PackageActiveConnectionsList activeConnectionsList);
    PackageSpecific(PackageUploadFile uploadFile);
    PackageSpecific(PackageFileNotFound fileNotFound);
    PackageSpecific(PackageReplicaManagerPing replicaManagerPing);
    PackageSpecific(PackageReplicaManagerPingResponse replicaManagerPingResponse);
    PackageSpecific(PackageReplicaManagerElectionElection replicaManagerElectionElection);
    PackageSpecific(PackageReplicaManagerElectionAnswer replicaManagerElectionAnswer);
    PackageSpecific(PackageReplicaManagerElectionCoordinator replicaManagerElectionCoordinator);
    PackageSpecific(PackageNewServerInfo newServerInfo);
    ~PackageSpecific();
};

// Pacote
struct alignas(ALIGN_VALUE) Package
{
    alignas(ALIGN_VALUE) PackageType package_type;
    alignas(ALIGN_VALUE) PackageSpecific package_specific;

    // Conversão de Package de representação local para be
    void htobe(void);
    // Conversão de Package de be para representação local
    void betoh(void);

    Package();
    Package(const Package &&rhs);
    Package &operator=(const Package &rhs);

    // Inicializa package_type dependendo do tipo do pacote base
    Package(PackageUserIdentification userIdentification);
    Package(PackageUserIdentificationResponse userIdentificationResponse);
    Package(PackageReplicaManagerIdentification replicaManagerIdentification);
    Package(PackageReplicaManagerIdentificationResponse replicaManagerIdentificationResponse);
    Package(PackageReplicaManagerTransferIdentification replicaManagerTransferIdentification);
    Package(PackageReplicaManagerTransferIdentificationResponse replicaManagerTransferIdentificationResponse);
    Package(PackageChangeEvent ChangeEvent);
    Package(PackageRequestFile requestFile);
    Package(PackageFileContent fileContent);
    Package(PackageRequestFileList requestFileList);
    Package(PackageFileList fileList);
    Package(PackageActiveConnectionsList activeConnectionsList);
    Package(PackageUploadFile file);
    Package(PackageFileNotFound fileNotFound);
    Package(PackageReplicaManagerPing replicaManagerPing);
    Package(PackageReplicaManagerPingResponse replicaManagerPingResponse);
    Package(PackageReplicaManagerElectionElection replicaManagerElectionElection);
    Package(PackageReplicaManagerElectionAnswer replicaManagerElectionAnswer);
    Package(PackageReplicaManagerElectionCoordinator replicaManagerElectionCoordinator);
    Package(PackageNewServerInfo newServerInfo);
};

#endif
