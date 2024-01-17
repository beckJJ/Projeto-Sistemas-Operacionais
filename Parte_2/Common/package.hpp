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
    INITAL_USER_INDENTIFICATION,
    // Resposta do servidor para identificação do usuario
    USER_INDENTIFICATION_RESPONSE,
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
    REPLICA_MANAGER_INDENTIFICATION_RESPONSE
};

// Indica aceitação ou rejeição da conexão do dispositivo
enum InitialUserIndentificationResponseStatus : char
{
    ACCEPTED,
    REJECTED,
};

// Indica aceitação ou rejeição da conexão do Replica Manager
enum InitialReplicaManagerIndentificationResponseStatus : char
{
    ACCEPTED,
    REJECTED,
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
struct alignas(ALIGN_VALUE) PackageUserIndentification
{
    // ID do dispositivo, usado caso connectionType == EVENT_CONNECTION
    alignas(ALIGN_VALUE) uint8_t deviceID;
    // Nome do usuário que deseja se conectar
    alignas(ALIGN_VALUE) char user_name[USER_NAME_MAX_LENGTH]{};

    PackageUserIndentification(uint8_t deviceID, const char _user_name[USER_NAME_MAX_LENGTH]);
};

// Resposta do servidor para identificação do usuário
struct alignas(ALIGN_VALUE) PackageUserIndentificationResponse
{
    // Indica se foi aceita ou rejeitada a conexão
    alignas(ALIGN_VALUE) InitialUserIndentificationResponseStatus status;
    // ID do dispositivo
    alignas(ALIGN_VALUE) uint8_t deviceID;

    PackageUserIndentificationResponse(InitialUserIndentificationResponseStatus status, uint8_t deviceID);
};

// Usado para que o Replica Manager se identifique
struct alignas(ALIGN_VALUE) PackageReplicaManagerIndentification
{
    // ID do RM, usado caso connectionType == EVENT_CONNECTION
    alignas(ALIGN_VALUE) uint8_t replicaManagerID;

    PackageReplicaManagerIndentification(uint8_t replicaManagerID);
};

// Resposta do servidor para identificação do Replica Manager
struct alignas(ALIGN_VALUE) PackageReplicaManagerIndentificationResponse
{
    // Indica se foi aceita ou rejeitada a conexão
    alignas(ALIGN_VALUE) ReplicaManagerIndentificationResponseStatus status;
    // ID do dispositivo
    alignas(ALIGN_VALUE) uint8_t replicaManagerIDID;

    PackageReplicaManagerIndentificationResponse(ReplicaManagerIndentificationResponseStatus status, uint8_t replicaManagerID);
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

    PackageChangeEvent();
    PackageChangeEvent(ChangeEvents event, uint8_t deviceID, const char _filename1[NAME_MAX], const char _filename2[NAME_MAX]);
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

// União de todos os pacotes
union alignas(ALIGN_VALUE) PackageSpecific
{
    alignas(ALIGN_VALUE) PackageUserIndentification userIdentification;
    alignas(ALIGN_VALUE) PackageUserIndentificationResponse userIdentificationResponse;
    alignas(ALIGN_VALUE) PackageReplicaManagerIndentification replicaManagerIdentification;
    alignas(ALIGN_VALUE) PackageReplicaManagerIndentificationResponse replicaManagerIdentificationResponse;
    alignas(ALIGN_VALUE) PackageChangeEvent changeEvent;
    alignas(ALIGN_VALUE) PackageRequestFile requestFile;
    alignas(ALIGN_VALUE) PackageFileContent fileContent;
    alignas(ALIGN_VALUE) PackageFileList fileList;
    alignas(ALIGN_VALUE) PackageRequestFileList requestFileList;
    alignas(ALIGN_VALUE) PackageUploadFile uploadFile;
    alignas(ALIGN_VALUE) PackageFileNotFound fileNotFound;

    PackageSpecific();
    PackageSpecific(PackageUserIndentification userIdentification);
    PackageSpecific(PackageUserIndentificationResponse userIdentificationResponse);
    PackageSpecific(PackageReplicaManagerIndentification replicaManagerIdentification);
    PackageSpecific(PackageReplicaManagerIndentificationResponse replicaManagerIdentificationResponse);  
    PackageSpecific(PackageChangeEvent ChangeEvent);
    PackageSpecific(PackageRequestFile requestFile);
    PackageSpecific(PackageFileContent fileContent);
    PackageSpecific(PackageRequestFileList requestFileList);
    PackageSpecific(PackageFileList fileList);
    PackageSpecific(PackageUploadFile uploadFile);
    PackageSpecific(PackageFileNotFound fileNotFound);
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
    Package(PackageUserIndentification userIdentification);
    Package(PackageUserIndentificationResponse userIdentificationResponse);
    Package(PackageReplicaManagerIndentification replicaManagerIdentification);
    Package(PackageReplicaManagerIndentificationResponse replicaManagerIdentificationResponse);
    Package(PackageChangeEvent ChangeEvent);
    Package(PackageRequestFile requestFile);
    Package(PackageFileContent fileContent);
    Package(PackageRequestFileList requestFileList);
    Package(PackageFileList fileList);
    Package(PackageUploadFile file);
    Package(PackageFileNotFound fileNotFound);
};

#endif
