sumppump: sumppump.o socket.o json.o sqlite3.o
	gcc7 -pthread -lssl -lcrypto -lm -o sumppump sumppump.o socket.o json.o sqlite3.o

sumppump.o: sumppump.c socket.h json.h sqlite3.h
	gcc7 -c sumppump.c

socket.o: socket.c
	gcc7 -c socket.c

json.o: json.c
	gcc7 -c json.c
	
sqlite3.o: sqlite3.c
	gcc7 -c sqlite3.c