CC:=gcc
CFLAGS:=-O2 -Wall -Wextra

default: all

all: myClient myServer

myClient:
	$(CC) $(CFLAGS) -o myClient myClient.c

myServer:
	$(CC) $(CFLAGS) -o myServer myServer.c

clean:
	rm myClient myServer
