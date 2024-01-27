#pragma once

#include <string>
#include <vector>
#include "../Common/package.hpp"

void create_client_dirs(std::vector<Connection_t> clientList, std::string base_path);
void *backupThread(void *arg);