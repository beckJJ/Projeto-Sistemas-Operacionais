CC:=g++
CFLAGS:=-O2 -Wall -Wextra

default: all

all: myClient myServer


myClient: Cliente/myClient.cpp auxiliaresCliente interfaceCliente comunicacaoCliente
	$(CC) $(CFLAGS) -o myClient Cliente/myClient.cpp Cliente/auxiliaresCliente.o Cliente/interfaceCliente.o Cliente/comunicacaoCliente.o

auxiliaresCliente: Cliente/auxiliaresCliente.cpp
	$(CC) $(CFLAGS) -c -o Cliente/auxiliaresCliente.o Cliente/auxiliaresCliente.cpp

interfaceCliente: Cliente/interfaceCliente.cpp
	$(CC) $(CFLAGS) -c -o Cliente/interfaceCliente.o Cliente/interfaceCliente.cpp

comunicacaoCliente: Cliente/comunicacaoCliente.cpp
	$(CC) $(CFLAGS) -c -o Cliente/comunicacaoCliente.o Cliente/comunicacaoCliente.cpp


myServer: Servidor/myServer.cpp auxiliaresServidor comunicacaoServidor servFunc
	$(CC) $(CFLAGS) -o myServer Servidor/myServer.cpp Servidor/servFunc.o Servidor/auxiliaresServidor.o Servidor/comunicacaoServidor.o

servFunc: Servidor/servFunc.cpp
	$(CC) $(CFLAGS) -c -o Servidor/servFunc.o Servidor/servFunc.cpp

auxiliaresServidor: Servidor/auxiliaresServidor.cpp
	$(CC) $(CFLAGS) -c -o Servidor/auxiliaresServidor.o Servidor/auxiliaresServidor.cpp

comunicacaoServidor: Servidor/comunicacaoServidor.cpp
	$(CC) $(CFLAGS) -c -o Servidor/comunicacaoServidor.o Servidor/comunicacaoServidor.cpp


clean:
	rm myClient myServer Cliente/*.o Servidor/*.o

