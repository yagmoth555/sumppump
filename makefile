sumppump: sumppump.o socket.o json.o sqlite3.o
	gcc7 -I/usr/local/include/mysql -L/usr/local/lib/mysql -lmysqlclient -lz -lm -lrt -lexecinfo -pthread -lssl -lcrypto -lm -lz -o sumppump sumppump.o socket.o json.o sqlite3.o

sumppump.o: sumppump.c socket.h json.h sqlite3.h
	gcc7 -I/usr/local/include/mysql -c sumppump.c 

socket.o: socket.c
	gcc7 -c socket.c

json.o: json.c
	gcc7 -c json.c
	
sqlite3.o: sqlite3.c
	gcc7 -c sqlite3.c