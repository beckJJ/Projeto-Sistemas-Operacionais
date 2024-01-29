#include "package.hpp"
#include <cstring>
#include <endian.h>
#include <iostream>

// Define construtores e destrutores para as estruturas

File::File() {}

Connection_t::Connection_t() {}

File::File(int64_t size, int64_t mtime, int64_t atime, int64_t ctime, const char _name[NAME_MAX])
    : size(size), mtime(mtime), atime(atime), ctime(ctime)
{
    strncpy(name, _name, NAME_MAX - 1);
}

bool File::operator<(const File &a) const
{
    return strcmp(name, a.name) < 0;
}

PackageUserIdentification::PackageUserIdentification(uint8_t deviceID, uint16_t listen_port, const char _user_name[USER_NAME_MAX_LENGTH])
    : deviceID(deviceID), listen_port(listen_port)
{
    strncpy(user_name, _user_name, USER_NAME_MAX_LENGTH - 1);
}

PackageUserIdentificationResponse::PackageUserIdentificationResponse(InitialUserIdentificationResponseStatus status, uint8_t deviceID)
    : status(status), deviceID(deviceID) {}

PackageReplicaManagerIdentification::PackageReplicaManagerIdentification(uint8_t deviceID)
    : deviceID(deviceID) {}

PackageReplicaManagerIdentificationResponse::PackageReplicaManagerIdentificationResponse(InitialReplicaManagerIdentificationResponseStatus status, uint8_t deviceID)
    : status(status), deviceID(deviceID) {}

PackageReplicaManagerTransferIdentification::PackageReplicaManagerTransferIdentification(uint8_t deviceID, uint16_t listen_port)
    : deviceID(deviceID), listen_port(listen_port) {}

PackageReplicaManagerTransferIdentificationResponse::PackageReplicaManagerTransferIdentificationResponse(ReplicaManagerTransferIdentificationResponseStatus status, uint8_t deviceID)
    : status(status), deviceID(deviceID) {}

PackageChangeEvent::PackageChangeEvent() : event((ChangeEvents)0), deviceID(0)
{
    filename1[0] = '\0';
    filename2[0] = '\0';
}

PackageChangeEvent::PackageChangeEvent(ChangeEvents event, uint8_t deviceID, const char _filename1[NAME_MAX], const char _filename2[NAME_MAX], const char _username[NAME_MAX])
    : event(event), deviceID(deviceID)
{
    strncpy(filename1, _filename1, NAME_MAX - 1);
    strncpy(filename2, _filename2, NAME_MAX - 1);
    strncpy(username , _username , NAME_MAX - 1);
}

PackageRequestFile::PackageRequestFile(const char _filename[NAME_MAX])
{
    strncpy(filename, _filename, NAME_MAX - 1);
}

PackageFileContent::PackageFileContent(int64_t size, uint16_t seqn, uint16_t data_length)
    : size(size), seqn(seqn), data_length(data_length) {}

PackageRequestFileList::PackageRequestFileList() {}

PackageFileList::PackageFileList(uint16_t count, uint16_t seqn, File file)
    : count(count), seqn(seqn), file(file) {}

Connection_t::Connection_t(uint16_t port, uint32_t host, uint16_t socket_id, const char _user_name[USER_NAME_MAX_LENGTH])
    : port(port), host(host), socket_id(socket_id)
{
    strncpy(user_name, _user_name, USER_NAME_MAX_LENGTH - 1);
}

PackageActiveConnectionsList::PackageActiveConnectionsList(uint16_t count, uint16_t seqn, bool is_client, Connection_t connection)
    : count(count), seqn(seqn), is_client(is_client), connection(connection) {}

PackageUploadFile::PackageUploadFile(File file) : File(file) {}

PackageFileNotFound::PackageFileNotFound() {}

PackageReplicaManagerPing::PackageReplicaManagerPing() {}

PackageReplicaManagerPingResponse::PackageReplicaManagerPingResponse() {}

PackageReplicaManagerElectionElection::PackageReplicaManagerElectionElection() {}

PackageReplicaManagerElectionAnswer::PackageReplicaManagerElectionAnswer() {}

PackageReplicaManagerElectionCoordinator::PackageReplicaManagerElectionCoordinator(uint8_t deviceID)
    : deviceID(deviceID) {}

PackageNewServerInfo::PackageNewServerInfo(uint16_t port, uint32_t host) 
    : port(port), host(host) {}

PackageCommitEvent::PackageCommitEvent() {}

PackageTransactionOK::PackageTransactionOK() {}


PackageSpecific::PackageSpecific() {}

PackageSpecific::PackageSpecific(PackageUserIdentification userIdentification)
    : userIdentification(userIdentification) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerIdentification replicaManagerIdentification)
    : replicaManagerIdentification(replicaManagerIdentification) {}

PackageSpecific::PackageSpecific(PackageUserIdentificationResponse userIdentificationResponse)
    : userIdentificationResponse(userIdentificationResponse) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerIdentificationResponse replicaManagerIdentificationResponse)
    : replicaManagerIdentificationResponse(replicaManagerIdentificationResponse) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerTransferIdentification replicaManagerTransferIdentification)
    : replicaManagerTransferIdentification(replicaManagerTransferIdentification) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerTransferIdentificationResponse replicaManagerTransferIdentificationResponse)
    : replicaManagerTransferIdentificationResponse(replicaManagerTransferIdentificationResponse) {}

PackageSpecific::PackageSpecific(PackageChangeEvent ChangeEvent)
    : changeEvent(ChangeEvent) {}

PackageSpecific::PackageSpecific(PackageRequestFile requestFile)
    : requestFile(requestFile) {}

PackageSpecific::PackageSpecific(PackageFileContent fileContent)
    : fileContent(fileContent) {}

PackageSpecific::PackageSpecific(PackageFileList fileList)
    : fileList(fileList) {}

PackageSpecific::PackageSpecific(PackageActiveConnectionsList activeConnectionsList)
    : activeConnectionsList(activeConnectionsList) {}

PackageSpecific::PackageSpecific(PackageRequestFileList requestFileList)
    : requestFileList(requestFileList) {}

PackageSpecific::PackageSpecific(PackageUploadFile uploadFile)
    : uploadFile(uploadFile) {}

PackageSpecific::PackageSpecific(PackageFileNotFound fileNotFound)
    : fileNotFound(fileNotFound) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerPing replicaManagerPing)
    : replicaManagerPing(replicaManagerPing) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerPingResponse replicaManagerPingResponse)
    : replicaManagerPingResponse(replicaManagerPingResponse) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerElectionElection replicaManagerElectionElection)
    : replicaManagerElectionElection(replicaManagerElectionElection) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerElectionAnswer replicaManagerElectionAnswer)
    : replicaManagerElectionAnswer(replicaManagerElectionAnswer) {}

PackageSpecific::PackageSpecific(PackageReplicaManagerElectionCoordinator replicaManagerElectionCoordinator)
    : replicaManagerElectionCoordinator(replicaManagerElectionCoordinator) {}

PackageSpecific::PackageSpecific(PackageNewServerInfo newServerInfo)
    : newServerInfo(newServerInfo) {}

PackageSpecific::PackageSpecific(PackageCommitEvent commitEvent)
    : commitEvent(commitEvent) {}

PackageSpecific::PackageSpecific(PackageTransactionOK transactionOK)
    : transactionOK(transactionOK) {}

PackageSpecific::~PackageSpecific() {}

Package::Package() {}

Package::Package(const Package &&rhs)
{
    package_type = rhs.package_type;

    // Move dependendo do tipo
    switch (package_type)
    {
    case INITIAL_USER_IDENTIFICATION:
        package_specific.userIdentification = std::move(rhs.package_specific.userIdentification);
        break;
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
        package_specific.replicaManagerIdentification = std::move(rhs.package_specific.replicaManagerIdentification);
        break;
    case USER_IDENTIFICATION_RESPONSE:
        package_specific.userIdentificationResponse = std::move(rhs.package_specific.userIdentificationResponse);
        break;
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
        package_specific.replicaManagerIdentificationResponse = std::move(rhs.package_specific.replicaManagerIdentificationResponse);
        break;
    case CHANGE_EVENT:
        package_specific.changeEvent = std::move(rhs.package_specific.changeEvent);
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
    case ACTIVE_CONNECTIONS_LIST:
        package_specific.activeConnectionsList = std::move(rhs.package_specific.activeConnectionsList);
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile = std::move(rhs.package_specific.uploadFile);
        break;
    case FILE_NOT_FOUND:
        package_specific.fileNotFound = std::move(rhs.package_specific.fileNotFound);
        break;
    case REPLICA_MANAGER_PING:
        package_specific.replicaManagerPing = std::move(rhs.package_specific.replicaManagerPing);
        break;
    case REPLICA_MANAGER_PING_RESPONSE:
        package_specific.replicaManagerPingResponse = std::move(rhs.package_specific.replicaManagerPingResponse);
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        package_specific.replicaManagerTransferIdentification = std::move(rhs.package_specific.replicaManagerTransferIdentification);
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
        package_specific.replicaManagerTransferIdentificationResponse = std::move(rhs.package_specific.replicaManagerTransferIdentificationResponse);
        break;
    case REPLICA_MANAGER_ELECTION_ELECTION:
        package_specific.replicaManagerElectionElection = std::move(rhs.package_specific.replicaManagerElectionElection);
        break;
    case REPLICA_MANAGER_ELECTION_ANSWER:
        package_specific.replicaManagerElectionAnswer = std::move(rhs.package_specific.replicaManagerElectionAnswer);
        break;
    case REPLICA_MANAGER_ELECTION_COORDINATOR:
        package_specific.replicaManagerElectionCoordinator = std::move(rhs.package_specific.replicaManagerElectionCoordinator);
        break;
    case NEW_SERVER_INFO:
        package_specific.newServerInfo = std::move(rhs.package_specific.newServerInfo);
        break;
    case COMMIT_EVENT:
        package_specific.commitEvent = std::move(rhs.package_specific.commitEvent);
        break;
    case TRANSACTION_OK:
        package_specific.transactionOK = std::move(rhs.package_specific.transactionOK);
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

    // Cópia dependendo do tipo
    switch (package_type)
    {
    case INITIAL_USER_IDENTIFICATION:
        package_specific.userIdentification = rhs.package_specific.userIdentification;
        break;
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
        package_specific.replicaManagerIdentification = rhs.package_specific.replicaManagerIdentification;
        break;
    case USER_IDENTIFICATION_RESPONSE:
        package_specific.userIdentificationResponse = rhs.package_specific.userIdentificationResponse;
        break;
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
        package_specific.replicaManagerIdentificationResponse = rhs.package_specific.replicaManagerIdentificationResponse;
        break;
    case CHANGE_EVENT:
        package_specific.changeEvent = rhs.package_specific.changeEvent;
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
    case ACTIVE_CONNECTIONS_LIST:
        package_specific.activeConnectionsList = rhs.package_specific.activeConnectionsList;
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile = rhs.package_specific.uploadFile;
        break;
    case FILE_NOT_FOUND:
        package_specific.fileNotFound = rhs.package_specific.fileNotFound;
        break;
    case REPLICA_MANAGER_PING:
        package_specific.replicaManagerPing = rhs.package_specific.replicaManagerPing;
        break;
    case REPLICA_MANAGER_PING_RESPONSE:
        package_specific.replicaManagerPingResponse = rhs.package_specific.replicaManagerPingResponse;
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        package_specific.replicaManagerTransferIdentification = rhs.package_specific.replicaManagerTransferIdentification;
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
        package_specific.replicaManagerTransferIdentificationResponse = rhs.package_specific.replicaManagerTransferIdentificationResponse;
        break;
    case REPLICA_MANAGER_ELECTION_ELECTION:
        package_specific.replicaManagerElectionElection = rhs.package_specific.replicaManagerElectionElection;
        break;
    case REPLICA_MANAGER_ELECTION_ANSWER:
        package_specific.replicaManagerElectionAnswer = rhs.package_specific.replicaManagerElectionAnswer;
        break;
    case REPLICA_MANAGER_ELECTION_COORDINATOR:
        package_specific.replicaManagerElectionCoordinator = rhs.package_specific.replicaManagerElectionCoordinator;
        break;
    case NEW_SERVER_INFO:
        package_specific.newServerInfo = rhs.package_specific.newServerInfo;
        break;
    case COMMIT_EVENT:
        package_specific.commitEvent = rhs.package_specific.commitEvent;
        break;
    case TRANSACTION_OK:
        package_specific.transactionOK = rhs.package_specific.transactionOK;
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }

    return *this;
}

Package::Package(PackageUserIdentification userIdentification)
    : package_type(INITIAL_USER_IDENTIFICATION), package_specific(userIdentification) {}

Package::Package(PackageReplicaManagerIdentification replicaManagerIdentification)
    : package_type(INITIAL_REPLICA_MANAGER_IDENTIFICATION), package_specific(replicaManagerIdentification) {}

Package::Package(PackageUserIdentificationResponse userIdentificationResponse)
    : package_type(USER_IDENTIFICATION_RESPONSE), package_specific(userIdentificationResponse) {}

Package::Package(PackageReplicaManagerIdentificationResponse replicaManagerIdentificationResponse)
    : package_type(REPLICA_MANAGER_IDENTIFICATION_RESPONSE), package_specific(replicaManagerIdentificationResponse) {}

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

Package::Package(PackageActiveConnectionsList activeConnectionsList)
    : package_type(ACTIVE_CONNECTIONS_LIST), package_specific(activeConnectionsList) {}

Package::Package(PackageUploadFile file)
    : package_type(UPLOAD_FILE), package_specific(file) {}

Package::Package(PackageFileNotFound fileNotFound)
    : package_type(FILE_NOT_FOUND), package_specific(fileNotFound) {}

Package::Package(PackageReplicaManagerPing replicaManagerPing)
    : package_type(REPLICA_MANAGER_PING), package_specific(replicaManagerPing) {}

Package::Package(PackageReplicaManagerPingResponse replicaManagerPingResponse)
    : package_type(REPLICA_MANAGER_PING_RESPONSE), package_specific(replicaManagerPingResponse) {}

Package::Package(PackageReplicaManagerTransferIdentification replicaManagerTransferIdentification)
    : package_type(REPLICA_MANAGER_TRANSFER_IDENTIFICATION), package_specific(replicaManagerTransferIdentification) {}

Package::Package(PackageReplicaManagerTransferIdentificationResponse replicaManagerTransferIdentificationResponse)
    : package_type(REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE), package_specific(replicaManagerTransferIdentificationResponse) {}

Package::Package(PackageReplicaManagerElectionElection replicaManagerElectionElection)
    : package_type(REPLICA_MANAGER_ELECTION_ELECTION), package_specific(replicaManagerElectionElection) {}

Package::Package(PackageReplicaManagerElectionAnswer replicaManagerElectionAnswer)
    : package_type(REPLICA_MANAGER_ELECTION_ANSWER), package_specific(replicaManagerElectionAnswer) {}

Package::Package(PackageReplicaManagerElectionCoordinator replicaManagerElectionCoordinator)
    : package_type(REPLICA_MANAGER_ELECTION_COORDINATOR), package_specific(replicaManagerElectionCoordinator) {}

Package::Package(PackageNewServerInfo newServerInfo)
    : package_type(NEW_SERVER_INFO), package_specific(newServerInfo) {}

Package::Package(PackageCommitEvent commitEvent)
    : package_type(COMMIT_EVENT), package_specific(commitEvent) {}

Package::Package(PackageTransactionOK transactionOK)
    : package_type(TRANSACTION_OK), package_specific(transactionOK) {}

// Conversão de Package de representação local para be
void Package::htobe(void)
{
    switch (this->package_type)
    {
    // Não há campo para converter
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
    case USER_IDENTIFICATION_RESPONSE:
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
    case CHANGE_EVENT:
    case REQUEST_FILE:
    case REQUEST_FILE_LIST:
    case REPLICA_MANAGER_PING:
    case REPLICA_MANAGER_PING_RESPONSE:
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
    case REPLICA_MANAGER_ELECTION_ELECTION:
    case REPLICA_MANAGER_ELECTION_ANSWER:
    case REPLICA_MANAGER_ELECTION_COORDINATOR:
    case COMMIT_EVENT:
    case TRANSACTION_OK:
    case FILE_NOT_FOUND:
        break;
    case INITIAL_USER_IDENTIFICATION:
        package_specific.userIdentification.listen_port = htobe16(package_specific.userIdentification.listen_port);
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
    case ACTIVE_CONNECTIONS_LIST:
        package_specific.activeConnectionsList.count = htobe16(package_specific.activeConnectionsList.count);
        package_specific.activeConnectionsList.seqn = htobe16(package_specific.activeConnectionsList.seqn);
        package_specific.activeConnectionsList.connection.port = htobe16(package_specific.activeConnectionsList.connection.port);
        package_specific.activeConnectionsList.connection.host = htobe32(package_specific.activeConnectionsList.connection.host);
        package_specific.activeConnectionsList.connection.socket_id = htobe16(package_specific.activeConnectionsList.connection.socket_id);
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile.size = htobe64(package_specific.uploadFile.size);
        package_specific.uploadFile.mtime = htobe64(package_specific.uploadFile.mtime);
        package_specific.uploadFile.atime = htobe64(package_specific.uploadFile.atime);
        package_specific.uploadFile.ctime = htobe64(package_specific.uploadFile.ctime);
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        package_specific.replicaManagerTransferIdentification.listen_port = htobe16(package_specific.replicaManagerTransferIdentification.listen_port);
        break;
    case NEW_SERVER_INFO:
        package_specific.newServerInfo.port = htobe16(package_specific.newServerInfo.port);
        package_specific.newServerInfo.host = htobe32(package_specific.newServerInfo.host);
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }
}

void Package::betoh(void)
{
    switch (this->package_type)
    {
    // Não há campo para converter
    case INITIAL_REPLICA_MANAGER_IDENTIFICATION:
    case USER_IDENTIFICATION_RESPONSE:
    case REPLICA_MANAGER_IDENTIFICATION_RESPONSE:
    case CHANGE_EVENT:
    case REQUEST_FILE:
    case REQUEST_FILE_LIST:
    case REPLICA_MANAGER_PING:
    case REPLICA_MANAGER_PING_RESPONSE:
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE:
    case REPLICA_MANAGER_ELECTION_ELECTION:
    case REPLICA_MANAGER_ELECTION_ANSWER:
    case REPLICA_MANAGER_ELECTION_COORDINATOR:
    case FILE_NOT_FOUND:
    case COMMIT_EVENT:
    case TRANSACTION_OK:
        break;
    case INITIAL_USER_IDENTIFICATION:
        package_specific.userIdentification.listen_port = be16toh(package_specific.userIdentification.listen_port);
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
    case ACTIVE_CONNECTIONS_LIST:
        package_specific.activeConnectionsList.count = be16toh(package_specific.activeConnectionsList.count);
        package_specific.activeConnectionsList.seqn = be16toh(package_specific.activeConnectionsList.seqn);
        package_specific.activeConnectionsList.connection.port = be16toh(package_specific.activeConnectionsList.connection.port);
        package_specific.activeConnectionsList.connection.host = be32toh(package_specific.activeConnectionsList.connection.host);
        package_specific.activeConnectionsList.connection.socket_id = be16toh(package_specific.activeConnectionsList.connection.socket_id);
        break;
    case UPLOAD_FILE:
        package_specific.uploadFile.size = be64toh(package_specific.uploadFile.size);
        package_specific.uploadFile.mtime = be64toh(package_specific.uploadFile.mtime);
        package_specific.uploadFile.atime = be64toh(package_specific.uploadFile.atime);
        package_specific.uploadFile.ctime = be64toh(package_specific.uploadFile.ctime);
        break;
    case REPLICA_MANAGER_TRANSFER_IDENTIFICATION:
        package_specific.replicaManagerTransferIdentification.listen_port = be16toh(package_specific.replicaManagerTransferIdentification.listen_port);
        break;
    case NEW_SERVER_INFO:
        package_specific.newServerInfo.port = be16toh(package_specific.newServerInfo.port);
        package_specific.newServerInfo.host = be32toh(package_specific.newServerInfo.host);
        break;
    default:
        throw std::invalid_argument("Unknown package_type.");
    }
}
