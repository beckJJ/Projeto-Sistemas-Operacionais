#pragma once

#include <optional>
#include "../Common/package.hpp"
#include "../Common/DadosConexao.hpp"
#include "../Common/package_functions.hpp"


std::optional<int> conecta_servidor(DadosConexao &dadosConexao);
int conecta_backup_main(DadosConexao &dadosConexao);
int conecta_backup_transfer_main(DadosConexao &dadosConexao, uint16_t listen_port);
void *pingThread(void *arg);