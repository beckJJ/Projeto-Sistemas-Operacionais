CC:=g++

# Opções para melhor diagnóstico e verificação de erros
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
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

CFLAGS:=$(HARDENING) -g -O2 -Wall -Wextra -pthread -std=c++17

# Configuração disponível por defines:
#   DEBUG_PACOTE:              Imprime pacotes recebidos e escritos em stderr
#   DEBUG_PACOTE_TID:          Exibe [tid: xxx] ao debugar pacotes
#   DEBUG_PACOTE_FILE_CONTENT: Imprime conteúdo do arquivo enviado no pacote em hex
#   LIMPA_TELA_NOOP:           Função limpatela() não limpará a tela

DEFINES:=-DDEBUG_PACOTE=false -DDEBUG_PACOTE_TID=false -DLIMPA_TELA_NOOP=false
# DEFINES:=-DDEBUG_PACOTE=true

default: all

all: myClient myServer


myClient: Cliente/myClient.cpp auxiliaresCliente interfaceCliente comunicacaoCliente package functions package_file package_functions eventThread readThread DadosConexao
	$(CC) $(CFLAGS) $(DEFINES) -o myClient Cliente/myClient.cpp Cliente/auxiliaresCliente.o Cliente/interfaceCliente.o Cliente/comunicacaoCliente.o Cliente/eventThread.o Cliente/readThread.o Cliente/DadosConexao.o Common/package.o Common/functions.o Common/package_file.o Common/package_functions.o

auxiliaresCliente: Cliente/auxiliaresCliente.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/auxiliaresCliente.o Cliente/auxiliaresCliente.cpp

interfaceCliente: Cliente/interfaceCliente.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/interfaceCliente.o Cliente/interfaceCliente.cpp

comunicacaoCliente: Cliente/comunicacaoCliente.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/comunicacaoCliente.o Cliente/comunicacaoCliente.cpp

eventThread: Cliente/eventThread.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/eventThread.o Cliente/eventThread.cpp

readThread: Cliente/readThread.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/readThread.o Cliente/readThread.cpp

DadosConexao: Cliente/DadosConexao.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Cliente/DadosConexao.o Cliente/DadosConexao.cpp


myServer: Servidor/myServer.cpp serverThread deviceManager serverLoop package functions package_file package_functions
	$(CC) $(CFLAGS) $(DEFINES) -o myServer Servidor/myServer.cpp Servidor/serverThread.o Servidor/deviceManager.o Servidor/serverLoop.o Common/package.o Common/functions.o Common/package_file.o Common/package_functions.o

deviceManager: Servidor/deviceManager.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Servidor/deviceManager.o Servidor/deviceManager.cpp

serverThread: Servidor/serverThread.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Servidor/serverThread.o Servidor/serverThread.cpp

serverLoop: Servidor/serverLoop.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Servidor/serverLoop.o Servidor/serverLoop.cpp


package: Common/package.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/package.o Common/package.cpp

functions: Common/functions.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/functions.o Common/functions.cpp

package_file: Common/package_file.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/package_file.o Common/package_file.cpp

package_functions: Common/package_functions.cpp
	$(CC) $(CFLAGS) $(DEFINES) -c -o Common/package_functions.o Common/package_functions.cpp

clean:
	rm -f myClient myServer Cliente/*.o Servidor/*.o
	rm -rf myClient.dSYM myServer.dSYM

