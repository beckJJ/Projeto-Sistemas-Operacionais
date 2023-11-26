CC:=g++

# As opções abaixo conflitam com -fsanitize=address:
#   -fsanitize=leak -fsanitize=thread

# HARDENING:=-D_FORTIFY_SOURCE=3 \
	-D_GLIBCXX_ASSERTIONS \
	-fPIE  -pie  -Wl,-z,relro,-z,now \
	-fstack-protector-strong \
	-fstack-clash-protection \
	-fcf-protection=full \
	-fstack-protector-all \
	-fstack-protector-strong \
	-fstack-clash-protection \
	-fsanitize=address \
	-fsanitize=undefined

CFLAGS:=$(HARDENING) -g -O2 -Wall -Wextra -pthread

# Configura defines:
#   DEBUG_PACOTE:              Imprime pacotes recebidos e escritos em stderr
#   DEBUG_PACOTE_FILE_CONTENT: Imprime conteúdo do arquivo enviado no pacote
#   LIMPA_TELA_NOOP:           limpatela() não mais irá limpar a tela

# DEFINES:=-DDEBUG_PACOTE=true -DLIMPA_TELA_NOOP=true
DEFINES:=-DDEBUG_PACOTE=true

default: all

all: myClient myServer


myClient: Cliente/myClient.cpp auxiliaresCliente interfaceCliente comunicacaoCliente package functions package_commands package_functions inotifyThread
	$(CC) $(CFLAGS) $(DEFINES) -o myClient Cliente/myClient.cpp Cliente/auxiliaresCliente.o Cliente/interfaceCliente.o Cliente/comunicacaoCliente.o Cliente/inotifyThread.o Common/package.o Common/functions.o Common/package_commands.o Common/package_functions.o

auxiliaresCliente: Cliente/auxiliaresCliente.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/auxiliaresCliente.o Cliente/auxiliaresCliente.cpp

interfaceCliente: Cliente/interfaceCliente.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/interfaceCliente.o Cliente/interfaceCliente.cpp

comunicacaoCliente: Cliente/comunicacaoCliente.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/comunicacaoCliente.o Cliente/comunicacaoCliente.cpp

inotifyThread: Cliente/inotifyThread.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/inotifyThread.o Cliente/inotifyThread.cpp


myServer: Servidor/myServer.cpp servFunc deviceManager package functions package_commands package_functions
	$(CC) $(CFLAGS) $(DEFINES) -o myServer Servidor/myServer.cpp Servidor/servFunc.o Servidor/deviceManager.o Common/package.o Common/functions.o Common/package_commands.o Common/package_functions.o

deviceManager: Servidor/deviceManager.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Servidor/deviceManager.o Servidor/deviceManager.cpp

servFunc: Servidor/servFunc.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Servidor/servFunc.o Servidor/servFunc.cpp


package: Common/package.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/package.o Common/package.cpp

functions: Common/functions.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/functions.o Common/functions.cpp

package_commands: Common/package_commands.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/package_commands.o Common/package_commands.cpp

package_functions: Common/package_functions.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/package_functions.o Common/package_functions.cpp

clean:
	rm -f myClient myServer Cliente/*.o Servidor/*.o
	rm -rf myClient.dSYM myServer.dSYM

