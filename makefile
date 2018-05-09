sumppump: sumppump.o socket.o
	gcc7 -pthread -lssl -lcrypto -o sumppump sumppump.o socket.o

sumppump.o: sumppump.c socket.h
	gcc7 -c sumppump.c

socket.o: socket.c
	gcc7 -c socket.c
