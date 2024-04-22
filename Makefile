CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all: server client

## server
server : server.o jeu.o network.o ipc.o: utils_v1.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o

server.o: server.c utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c server.c 

## client
client : client.o jeu.o network.o utils_v1.o
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o

client.o: client.c utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c client.c

## game 
jeu.o: jeu.c utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c jeu.c

## network
network.o: network.c utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c network.c

## ipc
ipc.o: ipc.c utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c ipc.c

## utils
utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c 

## clean
clear :
		clear

clean :
	rm -f *.o
	rm -f server
	rm -f client