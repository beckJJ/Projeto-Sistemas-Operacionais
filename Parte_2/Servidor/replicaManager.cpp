#include "replicaManager.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>

#include "../Common/package_functions.hpp"
#include "../Common/package.hpp"
#include "../Common/package_file.hpp"
#include "serverThread.hpp"

extern bool backup;
extern pthread_mutex_t backup_connection_lock; 

std::optional<int> conecta_servidor(DadosConexao &dadosConexao)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    /* Obtem informacao do host, a partir do seu endereco IP. */
    server = gethostbyname(dadosConexao.endereco_ip);

    if (server == NULL) {
        printf("Erro! Nenhum servidor com o IP %s foi encontrado!\n", dadosConexao.endereco_ip);
        return std::nullopt;
    }

    int socket_id;

    /* Inicia a utilizacao de um socket, guardando o valor inteiro que o referencia. */
    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Erro! Nao foi possivel iniciar utilizacao do socket do cliente!\n");
        return std::nullopt;
    }

    /* Identifica o protocolo de rede a ser utilizado. */
    serv_addr.sin_family = AF_INET;
    /* Identifica o numero de porta a ser utilizado. */
    serv_addr.sin_port = htons(atoi(dadosConexao.numero_porta));
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    /* Solicita abertura de conexao com o servidor. */
    if (connect(socket_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Erro! Nao foi possivel estabelecer conexao com o socket do servidor!\n");
        return std::nullopt;
    }
    return socket_id;
}

// Conecta thread de transfer do backup ao servidor principal
int conecta_backup_transfer_main(DadosConexao &dadosConexao)
{
    std::optional<int> socket_opt = conecta_servidor(dadosConexao);
    if (!socket_opt.has_value()) {
        return 1;
    }

    int current_socket = socket_opt.value();
    dadosConexao.socket_transfer = current_socket;

    Package package = Package(PackageReplicaManagerTransferIdentification(dadosConexao.deviceID_transfer, dadosConexao.listen_port));
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(current_socket, package, fileContentBuffer)) {
        printf("Nao foi possivel enviar pacote de identificacao para o servidor.\n");
        return 1;
    }

    if (read_package_from_socket(current_socket, package, fileContentBuffer)) {
        printf("Erro ao ler resposta inicial do servidor.\n");
        return 1;
    }
    
    // Resposta inválida
    if (package.package_type != REPLICA_MANAGER_TRANSFER_IDENTIFICATION_RESPONSE) {
        printf("Resposta invalida do servidor.\n");
        return 1;
    }

    // Dispositivo rejeitado
    if (package.package_specific.replicaManagerTransferIdentificationResponse.status == REJECTED_RM_T) {
        printf("Nao foi possivel se registrar como backup.\n");
        return 1;
    }

    dadosConexao.deviceID_transfer = package.package_specific.replicaManagerTransferIdentificationResponse.deviceID;
    return 0;
}

// Conecta backup ao servidor principal (função do backup) -> conexão da thread de ping
int conecta_backup_main(DadosConexao &dadosConexao)
{
    std::optional<int> socket_opt = conecta_servidor(dadosConexao);
    if (!socket_opt.has_value()) {
        return 1;
    }

    int current_socket = socket_opt.value();
    dadosConexao.socket_ping = current_socket;

    Package package = Package(PackageReplicaManagerIdentification(dadosConexao.deviceID_ping));
    std::vector<char> fileContentBuffer;

    if (write_package_to_socket(current_socket, package, fileContentBuffer)) {
        printf("Nao foi possivel enviar pacote de identificacao para o servidor.\n");
        return 1;
    }

    if (read_package_from_socket(current_socket, package, fileContentBuffer)) {
        printf("Erro ao ler resposta inicial do servidor.\n");
        return 1;
    }

    // Resposta inválida
    if (package.package_type != REPLICA_MANAGER_IDENTIFICATION_RESPONSE) {
        printf("Resposta invalida do servidor.\n");
        return 1;
    }

    // Dispositivo rejeitado
    if (package.package_specific.replicaManagerIdentificationResponse.status == REJECTED_RM) {
        printf("Nao foi possivel se registrar como backup.\n");
        return 1;
    }

    dadosConexao.deviceID_ping = package.package_specific.replicaManagerIdentificationResponse.deviceID;
    return 0;
}