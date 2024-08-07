
# Compilador
CC = gcc -I rawSocket -I utils -I funcServer -I funcClient
CFLAGS = -Wall -g

.PHONY: all clean purge dist

all: client server
OBJ = rawSocket/rawSocket.o utils/utils.o funcServer/funcServer.o funcClient/funcClient.o server.o client.o

clean:
	@echo "Limpando sujeira ..."
	@rm -f *~ *.bak

server: server.o rawSocket.o funcServer.o utils.o
	$(CC) -o server server.o rawSocket/rawSocket.o funcServer/funcServer.o utils/utils.o

client: client.o rawSocket.o funcClient.o utils.o
	$(CC) -o client client.o rawSocket/rawSocket.o funcClient/funcClient.o utils/utils.o

client.o: client.c
	$(CC) -c $(CFLAGS) client.c -o client.o

server.o: server.c
	$(CC) -c $(CFLAGS) server.c -o server.o

rawSocket.o: rawSocket/rawSocket.c rawSocket/rawSocket.h
	$(CC) -c $(CFLAGS) rawSocket/rawSocket.c -o rawSocket/rawSocket.o

utils.o: utils/utils.c utils/utils.h
	$(CC) -c $(CFLAGS) utils/utils.c -o utils/utils.o

funcServer.o: funcServer/funcServer.c funcServer/funcServer.h
	$(CC) -c $(CFLAGS) funcServer/funcServer.c -o funcServer/funcServer.o

funcClient.o: funcClient/funcClient.c funcClient/funcClient.h
	$(CC) -c $(CFLAGS) funcClient/funcClient.c -o funcClient/funcClient.o

removeObjects:
	-rm -f $(OBJ)

purge:  clean
	@echo "Limpando tudo ..."
	@rm -f *.o core a.out
