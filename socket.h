// * sumppump/socket.h
// * Copyright (c) 2018 Philippe Levesque <EMail: yagmoth555@yahoo.com>
// * https://github.com/yagmoth555/sumppump

//---------------------------------------------------------------------------
#ifndef   socket_classH
#define   socket_classH
//---------------------------------------------------------------------------
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <stdbool.h>
#include "global.h"	
//---------------------------------------------------------------------------
#define 		CA_LIST 					"root.pem"
#define 		HOST						"localhost"
#define 		RANDOM  					"random.pem"
#define 		PORT						443
#define 		BUFSIZZ 					1024
#define 		KEYFILE 					"client.pem"
#define 		PASSWORD 					"password"
#define			G_INIMINPAGESIZE			1000000
//---------------------------------------------------------------------------
void SOCK_init(T_SOCK *tSOCK);
void SOCK_delete(T_SOCK *tSOCK);
void SOCK_SetReady(T_SOCK *tSOCK, _Bool value);
void SOCK_InitSSLCTX(T_SOCK *tSOCK, char *keyfile, char *password);
void SOCK_DestroyCTX(T_SOCK *tSOCK);
void SOCK_SSLCheckCert(T_SOCK *tSOCK);
int SOCK_ConnectSocket(T_SOCK *tSOCK);
char *SOCK_readLine(T_SOCK *tSOCK);
_Bool SOCK_IsReady(T_SOCK *tSOCK);
int SOCK_Connect(T_SOCK *tSOCK);
int SOCK_send(T_SOCK *tSOCK, int param, char buf[1024]);
int SOCK_recv(T_SOCK *tSOCK);
int SOCK_HTTPGetStatus(T_SOCK *tSOCK);
int SOCK_HTTPNTLMAuth(T_SOCK *tSOCK);
//---------------------------------------------------------------------------
#endif