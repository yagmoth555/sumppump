sumppump: sumppump.o socket.o json.o
	gcc7 -pthread -lssl -lcrypto -lm -o sumppump sumppump.o socket.o json.o

sumppump.o: sumppump.c socket.h json.h
	gcc7 -c sumppump.c

socket.o: socket.c
	gcc7 -c socket.c

json.o: json.c
	gcc7 -c json.c