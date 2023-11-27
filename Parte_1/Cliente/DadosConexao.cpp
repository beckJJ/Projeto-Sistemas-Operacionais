#include "DadosConexao.hpp"

DadosConexao::DadosConexao()
{
    main_connection_socket = -1;
    event_connection_socket = -1;

    main_connection_socket_lock = new pthread_mutex_t;
    event_connection_socket_lock = new pthread_mutex_t;

    pthread_mutex_init(main_connection_socket_lock, NULL);
    pthread_mutex_init(event_connection_socket_lock, NULL);
}

DadosConexao::~DadosConexao()
{
    pthread_mutex_destroy(main_connection_socket_lock);
    pthread_mutex_destroy(event_connection_socket_lock);

    delete main_connection_socket_lock;
    delete event_connection_socket_lock;
}
