#include "DadosConexao.hpp"

DadosConexao::DadosConexao()
{
    socket = -1;

    nome_usuario[0] = 0;
    endereco_ip[0]  = 0;
    numero_porta[0] = 0;

    socket_lock = new pthread_mutex_t;
    file_list_lock = new pthread_mutex_t;
    file_list_cond = new pthread_cond_t;

    pthread_mutex_init(socket_lock, NULL);
    pthread_mutex_init(file_list_lock, NULL);
    pthread_cond_init(file_list_cond, NULL);
}

DadosConexao::~DadosConexao()
{
    pthread_mutex_destroy(socket_lock);
    pthread_mutex_destroy(file_list_lock);
    pthread_cond_destroy(file_list_cond);

    delete socket_lock;
    delete file_list_lock;
    delete file_list_cond;
}
