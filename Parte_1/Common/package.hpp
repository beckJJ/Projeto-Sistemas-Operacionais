#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include <cstdint>
#include <vector>
#include <climits>

// Define align used by struct and union
#define ALIGN_VALUE 8

#define MAX_DATA_SIZE 4096

#define USER_NAME_MAX_LENGTH 128

enum PackageType : char
{
    INITAL_USER_INDENTIFICATION,
    USER_INDENTIFICATION_RESPONSE,
    CHANGE_EVENT,
    REQUEST_FILE,
    FILE_CONTENT,
    REQUEST_FILE_LIST,
    FILE_LIST,
    UPLOAD_FILE,
    FILE_NOT_FOUND,
};

enum InitialUserIndentificationResponseStatus : char
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
};

struct alignas(ALIGN_VALUE) File
{
    // Tamanho do arquivo/nodo
    alignas(ALIGN_VALUE) int64_t size = 0;
    // MAC-time
    alignas(ALIGN_VALUE) int64_t mtime = 0;
    alignas(ALIGN_VALUE) int64_t atime = 0;
    alignas(ALIGN_VALUE) int64_t ctime = 0;
    // Nome do arquivo
    alignas(ALIGN_VALUE) char name[NAME_MAX]{};

    File();
    File(int64_t size, int64_t mtime, int64_t atime, int64_t ctime, const char _name[NAME_MAX]);
};

struct alignas(ALIGN_VALUE) PackageUserIndentification
{
    alignas(ALIGN_VALUE) char user_name[USER_NAME_MAX_LENGTH]{};

    PackageUserIndentification(const char _user_name[USER_NAME_MAX_LENGTH]);
};

struct alignas(ALIGN_VALUE) PackageUserIndentificationResponse
{
    alignas(ALIGN_VALUE) InitialUserIndentificationResponseStatus status;
    alignas(ALIGN_VALUE) int8_t deviceID;

    PackageUserIndentificationResponse(InitialUserIndentificationResponseStatus status, int8_t deviceID);
};

struct alignas(ALIGN_VALUE) PackageChangeEvent
{
    alignas(ALIGN_VALUE) ChangeEvents event;
    alignas(ALIGN_VALUE) char filename[NAME_MAX]{};

    PackageChangeEvent(ChangeEvents event, const char _filename[NAME_MAX]);
};

struct alignas(ALIGN_VALUE) PackageRequestFile
{
    alignas(ALIGN_VALUE) char filename[NAME_MAX]{};

    PackageRequestFile(const char _filename[NAME_MAX]);
};

struct alignas(ALIGN_VALUE) PackageFileContentBase
{
    alignas(ALIGN_VALUE) int64_t size;
    alignas(ALIGN_VALUE) uint16_t seqn;
    alignas(ALIGN_VALUE) uint16_t data_length;

    PackageFileContentBase(int64_t size, uint16_t seqn, uint16_t data_length);
};

struct alignas(ALIGN_VALUE) PackageFileContent : PackageFileContentBase
{
    PackageFileContent(PackageFileContentBase fileContentBase);
};

struct alignas(ALIGN_VALUE) PackageRequestFileList
{
    PackageRequestFileList();
};

struct alignas(ALIGN_VALUE) PackageFileList
{
    alignas(ALIGN_VALUE) uint16_t count;
    alignas(ALIGN_VALUE) uint16_t seqn;
    alignas(ALIGN_VALUE) File file;

    PackageFileList(uint16_t count, uint16_t seqn, File file);
};

struct alignas(ALIGN_VALUE) PackageUploadFile : File
{
    PackageUploadFile(File file);
};

struct alignas(ALIGN_VALUE) PackageFileNotFound
{
    PackageFileNotFound();
};

union alignas(ALIGN_VALUE) PackageSpecific
{
    alignas(ALIGN_VALUE) PackageUserIndentification userIdentification;
    alignas(ALIGN_VALUE) PackageUserIndentificationResponse userIdentificationResponse;
    alignas(ALIGN_VALUE) PackageChangeEvent ChangeEvent;
    alignas(ALIGN_VALUE) PackageRequestFile requestFile;
    alignas(ALIGN_VALUE) PackageFileContent fileContent;
    alignas(ALIGN_VALUE) PackageFileList fileList;
    alignas(ALIGN_VALUE) PackageRequestFileList requestFileList;
    alignas(ALIGN_VALUE) PackageUploadFile uploadFile;
    alignas(ALIGN_VALUE) PackageFileNotFound fileNotFound;

    PackageSpecific();
    PackageSpecific(PackageUserIndentification userIdentification);
    PackageSpecific(PackageUserIndentificationResponse userIdentificationResponse);
    PackageSpecific(PackageChangeEvent ChangeEvent);
    PackageSpecific(PackageRequestFile requestFile);
    PackageSpecific(PackageFileContent fileContent);
    PackageSpecific(PackageRequestFileList requestFileList);
    PackageSpecific(PackageFileList fileList);
    PackageSpecific(PackageUploadFile uploadFile);
    PackageSpecific(PackageFileNotFound fileNotFound);
    ~PackageSpecific();
};

// PackageError

struct alignas(ALIGN_VALUE) Package
{
    alignas(ALIGN_VALUE) PackageType package_type;
    alignas(ALIGN_VALUE) PackageSpecific package_specific;

    // Funções de conversão de Package em representação local para big-endian
    void htobe(void);
    void betoh(void);

    Package();
    Package(const Package &&rhs);
    Package &operator=(const Package &rhs);
    Package(PackageUserIndentification userIdentification);
    Package(PackageUserIndentificationResponse userIdentificationResponse);
    Package(PackageChangeEvent ChangeEvent);
    Package(PackageRequestFile requestFile);
    Package(PackageFileContent fileContent);
    Package(PackageRequestFileList requestFileList);
    Package(PackageFileList fileList);
    Package(PackageUploadFile file);
    Package(PackageFileNotFound fileNotFound);
};

#endif
