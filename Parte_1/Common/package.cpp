#include "package.hpp"
#include <cstring>
#include <endian.h>
#include <iostream>

File::File() {}

File::File(int64_t size, int64_t mtime, int64_t atime, int64_t ctime, const char _name[NAME_MAX])
    : size(size), mtime(mtime), atime(atime), ctime(ctime)
{
    strncpy(name, _name, NAME_MAX - 1);
}

PackageUserIndentification::PackageUserIndentification(const char _user_name[USER_NAME_MAX_LENGTH])
{
    strncpy(user_name, _user_name, USER_NAME_MAX_LENGTH - 1);
}

PackageUserIndentificationResponse::PackageUserIndentificationResponse(InitialUserIndentificationResponseStatus status, int8_t deviceID)
    : status(status), deviceID(deviceID) {}

PackageChangeEvent::PackageChangeEvent(ChangeEvents event, const char _filename[NAME_MAX])
    : event(event)
{
    strncpy(filename, _filename, NAME_MAX - 1);
}

PackageRequestFile::PackageRequestFile(const char _filename[NAME_MAX])
{
    strncpy(filename, _filename, NAME_MAX - 1);
}

PackageFileContentBase::PackageFileContentBase(int64_t size, uint16_t seqn, uint16_t data_length)
    : size(size), seqn(seqn), data_length(data_length) {}

PackageFileContent::PackageFileContent(PackageFileContentBase fileContentBase)
    : PackageFileContentBase(fileContentBase) {}

PackageRequestFileList::PackageRequestFileList() {}

PackageFileList::PackageFileList(uint16_t count, uint16_t seqn, File file)
    : count(count), seqn(seqn), file(file) {}

PackageUploadFile::PackageUploadFile(File file) : File(file) {}

PackageFileNotFound::PackageFileNotFound() {}

PackageSpecific::PackageSpecific() {}

PackageSpecific::PackageSpecific(PackageUserIndentification userIdentification)
    : userIdentification(userIdentification) {}

PackageSpecific::PackageSpecific(PackageUserIndentificationResponse userIdentificationResponse)
    : userIdentificationResponse(userIdentificationResponse) {}

PackageSpecific::PackageSpecific(PackageChangeEvent ChangeEvent)
    : ChangeEvent(ChangeEvent) {}

PackageSpecific::PackageSpecific(PackageRequestFile requestFile)
    : requestFile(requestFile) {}

PackageSpecific::PackageSpecific(PackageFileContent fileContent)
    : fileContent(fileContent) {}

PackageSpecific::PackageSpecific(PackageFileList fileList)
    : fileList(fileList) {}

PackageSpecific::PackageSpecific(PackageRequestFileList requestFileList)
    : requestFileList(requestFileList) {}

PackageSpecific::PackageSpecific(PackageUploadFile uploadFile)
    : uploadFile(uploadFile) {}

PackageSpecific::PackageSpecific(PackageFileNotFound fileNotFound)
    : fileNotFound(fileNotFound) {}

PackageSpecific::~PackageSpecific() {}

Package::Package() {}

Package::Package(const Package &&rhs)
{
    package_type = rhs.package_type;

    switch (package_type)
    {
    case INITAL_USER_INDENTIFICATION:
        package_specific.userIdentification = std::move(rhs.package_specific.userIdentification);
        break;
    case USER_INDENTIFICATION_RESPONSE:
        package_specific.userIdentificationResponse = std::move(rhs.package_specific.userIdentificationResponse);
        break;
    case CHANGE_EVENT:
        package_specific.ChangeEvent = std::move(rhs.package_specific.ChangeEvent);
        break;
    case REQUEST_FILE:
        package_specific.requestFile = std::move(rhs.package_specific.requestFile);
        break;
    case FILE_CONTENT:
        package_specific.fileContent = std::move(rhs.package_specific.fileContent);
        break;
    case REQUEST_FILE_LIST:
        package_specific.requestFileList = std::move(rhs.package_specific.requestFileList);
        break;
    case FILE_LIST:
        package_specific.fileList = std::move(rhs.package_specific.fileList);
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile = std::move(rhs.package_specific.uploadFile);
        break;
    case FILE_NOT_FOUND:
        package_specific.fileNotFound = std::move(rhs.package_specific.fileNotFound);
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }
}
Package &Package::operator=(const Package &rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    package_type = rhs.package_type;

    switch (package_type)
    {
    case INITAL_USER_INDENTIFICATION:
        package_specific.userIdentification = rhs.package_specific.userIdentification;
        break;
    case USER_INDENTIFICATION_RESPONSE:
        package_specific.userIdentificationResponse = rhs.package_specific.userIdentificationResponse;
        break;
    case CHANGE_EVENT:
        package_specific.ChangeEvent = rhs.package_specific.ChangeEvent;
        break;
    case REQUEST_FILE:
        package_specific.requestFile = rhs.package_specific.requestFile;
        break;
    case FILE_CONTENT:
        package_specific.fileContent = rhs.package_specific.fileContent;
        break;
    case REQUEST_FILE_LIST:
        package_specific.requestFileList = rhs.package_specific.requestFileList;
        break;
    case FILE_LIST:
        package_specific.fileList = rhs.package_specific.fileList;
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile = rhs.package_specific.uploadFile;
        break;
    case FILE_NOT_FOUND:
        package_specific.fileNotFound = rhs.package_specific.fileNotFound;
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }

    return *this;
}

Package::Package(PackageUserIndentification userIdentification)
    : package_type(INITAL_USER_INDENTIFICATION), package_specific(userIdentification) {}

Package::Package(PackageUserIndentificationResponse userIdentificationResponse)
    : package_type(USER_INDENTIFICATION_RESPONSE), package_specific(userIdentificationResponse) {}

Package::Package(PackageChangeEvent ChangeEvent)
    : package_type(CHANGE_EVENT), package_specific(ChangeEvent) {}

Package::Package(PackageRequestFile requestFile)
    : package_type(REQUEST_FILE), package_specific(requestFile) {}

Package::Package(PackageFileContent fileContent)
    : package_type(FILE_CONTENT), package_specific(fileContent) {}

Package::Package(PackageRequestFileList requestFileList)
    : package_type(REQUEST_FILE_LIST), package_specific(requestFileList) {}

Package::Package(PackageFileList fileList)
    : package_type(FILE_LIST), package_specific(fileList) {}

Package::Package(PackageUploadFile file)
    : package_type(UPLOAD_FILE), package_specific(file) {}

Package::Package(PackageFileNotFound fileNotFound)
    : package_type(FILE_NOT_FOUND), package_specific(fileNotFound) {}

void Package::htobe(void)
{
    switch (this->package_type)
    {
    case INITAL_USER_INDENTIFICATION:
    case USER_INDENTIFICATION_RESPONSE:
    case CHANGE_EVENT:
    case REQUEST_FILE:
    case REQUEST_FILE_LIST:
    case FILE_NOT_FOUND:
        break;
    case FILE_CONTENT:
        package_specific.fileContent.size = htobe64(package_specific.fileContent.size);
        package_specific.fileContent.seqn = htobe16(package_specific.fileContent.seqn);
        package_specific.fileContent.data_length = htobe16(package_specific.fileContent.data_length);
        break;
    case FILE_LIST:
        package_specific.fileList.count = htobe16(package_specific.fileList.count);
        package_specific.fileList.seqn = htobe16(package_specific.fileList.seqn);
        package_specific.fileList.file.size = htobe64(package_specific.fileList.file.size);
        package_specific.fileList.file.mtime = htobe64(package_specific.fileList.file.mtime);
        package_specific.fileList.file.atime = htobe64(package_specific.fileList.file.atime);
        package_specific.fileList.file.ctime = htobe64(package_specific.fileList.file.ctime);
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile.size = htobe64(package_specific.uploadFile.size);
        package_specific.uploadFile.mtime = htobe64(package_specific.uploadFile.mtime);
        package_specific.uploadFile.atime = htobe64(package_specific.uploadFile.atime);
        package_specific.uploadFile.ctime = htobe64(package_specific.uploadFile.ctime);
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }
}

void Package::betoh(void)
{
    switch (this->package_type)
    {
    case INITAL_USER_INDENTIFICATION:
    case USER_INDENTIFICATION_RESPONSE:
    case CHANGE_EVENT:
    case REQUEST_FILE:
    case REQUEST_FILE_LIST:
    case FILE_NOT_FOUND:
        break;
    case FILE_CONTENT:
        package_specific.fileContent.size = be64toh(package_specific.fileContent.size);
        package_specific.fileContent.seqn = be16toh(package_specific.fileContent.seqn);
        package_specific.fileContent.data_length = be16toh(package_specific.fileContent.data_length);
        break;
    case FILE_LIST:
        package_specific.fileList.count = be16toh(package_specific.fileList.count);
        package_specific.fileList.seqn = be16toh(package_specific.fileList.seqn);
        package_specific.fileList.file.size = be64toh(package_specific.fileList.file.size);
        package_specific.fileList.file.mtime = be64toh(package_specific.fileList.file.mtime);
        package_specific.fileList.file.atime = be64toh(package_specific.fileList.file.atime);
        package_specific.fileList.file.ctime = be64toh(package_specific.fileList.file.ctime);
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile.size = be64toh(package_specific.uploadFile.size);
        package_specific.uploadFile.mtime = be64toh(package_specific.uploadFile.mtime);
        package_specific.uploadFile.atime = be64toh(package_specific.uploadFile.atime);
        package_specific.uploadFile.ctime = be64toh(package_specific.uploadFile.ctime);
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }
}
