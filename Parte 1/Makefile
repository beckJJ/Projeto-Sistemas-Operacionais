CC:=gcc
CFLAGS:=-O2 -Wall -Wextra

default: all

all: myClient myServer


myClient: Cliente/myClient.c auxiliaresCliente interfaceCliente comunicacaoCliente
	$(CC) $(CFLAGS) -o myClient Cliente/myClient.c Cliente/auxiliaresCliente.o Cliente/interfaceCliente.o Cliente/comunicacaoCliente.o

auxiliaresCliente: Cliente/auxiliaresCliente.c
	$(CC) $(CFLAGS) -c -o Cliente/auxiliaresCliente.o Cliente/auxiliaresCliente.c

interfaceCliente: Cliente/interfaceCliente.c
	$(CC) $(CFLAGS) -c -o Cliente/interfaceCliente.o Cliente/interfaceCliente.c

comunicacaoCliente: Cliente/comunicacaoCliente.c
	$(CC) $(CFLAGS) -c -o Cliente/comunicacaoCliente.o Cliente/comunicacaoCliente.c


myServer: Servidor/myServer.c auxiliaresServidor comunicacaoServidor
	$(CC) $(CFLAGS) -o myServer Servidor/myServer.c Servidor/auxiliaresServidor.o Servidor/comunicacaoServidor.o

auxiliaresServidor: Servidor/auxiliaresServidor.c
	$(CC) $(CFLAGS) -c -o Servidor/auxiliaresServidor.o Servidor/auxiliaresServidor.c

comunicacaoServidor: Servidor/comunicacaoServidor.c
	$(CC) $(CFLAGS) -c -o Servidor/comunicacaoServidor.o Servidor/comunicacaoServidor.c


clean:
	rm myClient myServer Cliente/*.o Servidor/*.o

