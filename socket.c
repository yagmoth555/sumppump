// * sumppump/socket.c
// * Copyright (c) 2018 Philippe Levesque <EMail: yagmoth555@yahoo.com>
// * https://github.com/yagmoth555/sumppump

//---------------------------------------------------------------------------
#include "socket.h"



//---------------------------------------------------------------------------
// SOCK::SOCK
//---------------------------------------------------------------------------
void SOCK_init(T_SOCK *tSOCK)
{
	tSOCK->ssl=NULL;
	tSOCK->cCode = (char *) malloc(G_INIMINPAGESIZE);
	ZeroMemory(tSOCK->cCode, G_INIMINPAGESIZE);
	SOCK_SetReady(tSOCK, false);
	SOCK_InitSSLCTX(tSOCK, KEYFILE, PASSWORD);
}

//---------------------------------------------------------------------------
// SOCK::~SOCK
//---------------------------------------------------------------------------
void SOCK_delete(T_SOCK *tSOCK)
{
	if (tSOCK->ctx)
		SOCK_DestroyCTX(tSOCK);
	if (tSOCK->ssl)
		SSL_shutdown((SSL *)tSOCK->ssl);
	if (tSOCK->ssl)
		SSL_free((SSL *)tSOCK->ssl);
}

//---------------------------------------------------------------------------
// SOCK_SetReady
//---------------------------------------------------------------------------
void SOCK_SetReady(T_SOCK *tSOCK, _Bool value)
{
	tSOCK->Connected = value;
}

//---------------------------------------------------------------------------
// SOCK::IsReady
//---------------------------------------------------------------------------
_Bool SOCK_IsReady(T_SOCK *tSOCK)
{
	if (tSOCK->Connected)
		return true;
	return false;
}


//---------------------------------------------------------------------------
// SOCK::InitSSLCTX
//---------------------------------------------------------------------------
void SOCK_InitSSLCTX(T_SOCK *tSOCK, char *keyfile, char *password)
{
	const SSL_METHOD *meth;

	/* Global system initialization*/
	SSL_library_init();
	SSL_load_error_strings();

	/* Create our context*/
	meth=SSLv23_method();

	// need to check ctx for NULL...
	tSOCK->ctx=SSL_CTX_new(meth);

	/* Load our keys and certificates*/
	/*
	if(!(SSL_CTX_use_certificate_chain_file(ctx, keyfile)))
		cfg_log(L"Can't read certificate file");
	pass=password;
	SSL_CTX_set_default_passwd_cb(ctx, SSLPasswordCB);
	if(!(SSL_CTX_use_PrivateKey_file(ctx, keyfile,SSL_FILETYPE_PEM)))
		cfg_log(L"Can't read key file");

	/* Load the CAs we trust */ /*
	if(!(SSL_CTX_load_verify_locations(ctx, CA_LIST,0)))
		cfg_log(L"Can't read CA list");
	#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
	SSL_CTX_set_verify_depth(ctx,1);
	#endif
	*/
	//return ctx;
 }

//---------------------------------------------------------------------------
// SOCK::DestroyCTX
//---------------------------------------------------------------------------
void SOCK_DestroyCTX(T_SOCK *tSOCK)
{
	if (tSOCK->ctx) SSL_CTX_free(tSOCK->ctx);
}

//---------------------------------------------------------------------------
// SOCK::SSLPasswordCB
//---------------------------------------------------------------------------
/*The password code is not thread safe*/
static int SOCK_SSLPasswordCB(T_SOCK *tSOCK, char *buf,int num, int rwflag,void *userdata)
{
	if(num<(int)strlen(tSOCK->pass)+1)
		return(0);

	strcpy(buf,tSOCK->pass);
	return(strlen(tSOCK->pass));
}

//---------------------------------------------------------------------------
// SOCK::Connect
//---------------------------------------------------------------------------
int SOCK_Connect(T_SOCK *tSOCK)
{
	int require_server_auth=0;
	int nRet = 0;
	int err = 0;

	/* Connect the TCP socket*/
	if(SOCK_ConnectSocket(tSOCK)==0) {
		printf(ANSI_COLOR_YELLOW	"* ConnectSocket error\n"	ANSI_COLOR_RESET);
		return 0;
	}

	/* Connect the SSL socket */
	tSOCK->ssl=SSL_new(tSOCK->ctx);
	if (tSOCK->ssl==NULL) {
		printf(ANSI_COLOR_YELLOW	"* SSL_new() error\n"	ANSI_COLOR_RESET);
		return 0;
	}

	//SSL_set_connect_state(ssl);
	//sbio=BIO_new_connect("win2k3-r2-std:443");

	tSOCK->sbio=BIO_new_socket(tSOCK->sock,BIO_NOCLOSE);
	if (tSOCK->sbio==NULL) {
		printf(ANSI_COLOR_YELLOW	"* BIO_new_socket() error\n"	ANSI_COLOR_RESET);
		return 0;
	}

	//BIO_set_nbio(sbio,1);
	/* SSL_set_verify(conn->ssl, 0, NULL); */
	/* set read/write BIO layer to non-blocking mode */

	SSL_set_bio(tSOCK->ssl,tSOCK->sbio,tSOCK->sbio);
	/* set accept state , SSL-Handshake will be handled transparent while SSL_[read|write] */
	//SSL_set_connect_state(ssl);

	//if(BIO_do_connect(sbio) <= 0) {
	/* Handle failed connection */
	//}

	nRet = SSL_connect(tSOCK->ssl);
	if(nRet<=0) {
		printf(ANSI_COLOR_YELLOW	"* SSL connect error\n"	ANSI_COLOR_RESET);
		err = SSL_get_error(tSOCK->ssl, nRet);
		if (err==SSL_ERROR_ZERO_RETURN)
			printf(ANSI_COLOR_YELLOW	"* SSL_ERROR_ZERO_RETURN\n"	ANSI_COLOR_RESET);
		else if (err==SSL_ERROR_WANT_X509_LOOKUP)
			printf(ANSI_COLOR_YELLOW	"* SSL_ERROR_WANT_X509_LOOKUP\n"	ANSI_COLOR_RESET);
		else if (err==SSL_ERROR_SYSCALL)
			printf(ANSI_COLOR_YELLOW	"* SSL_ERROR_SYSCALL\n"	ANSI_COLOR_RESET);
		else if (err==SSL_ERROR_WANT_CONNECT)
			printf(ANSI_COLOR_YELLOW	"* SSL_ERROR_WANT_CONNECT\n"	ANSI_COLOR_RESET);

		return 0;
	}

	//SSL_do_handshake(ssl);

	if(require_server_auth)
		SOCK_SSLCheckCert(tSOCK);

	SOCK_SetReady(tSOCK, true);
	return 1;
}

//---------------------------------------------------------------------------
// SOCK::ConnectSocket
//---------------------------------------------------------------------------
int SOCK_ConnectSocket(T_SOCK *tSOCK)
{
	struct hostent *hp;
	struct sockaddr_in addr;
	u_long iMode = 1; // 0 = blocking - 1 = non-blocking

	if(!(hp=gethostbyname(tSOCK->cHost))) {
		printf(ANSI_COLOR_RED	"* Couldn't resolve host\n"	ANSI_COLOR_RESET);
		return 0;
	}
	memset(&addr,0,sizeof(addr));
	addr.sin_addr=*(struct in_addr*)
	hp->h_addr_list[0];
	addr.sin_family=AF_INET;
	addr.sin_port=htons(tSOCK->iPort);

	if((tSOCK->sock=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP))<0) {
		printf(ANSI_COLOR_RED	"* Couldn't create socket\n"	ANSI_COLOR_RESET);
		return 0;
	}
	if(connect(tSOCK->sock,(struct sockaddr *)&addr, sizeof(addr))<0) {
		printf(ANSI_COLOR_RED	"* Couldn't connect socket\n"	ANSI_COLOR_RESET);
		return 0;
	}
	
	//fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK); 
	/*if ((ioctlsocket(sock, FIONBIO, &iMode)) != NO_ERROR) {
		printf("ioctlsocket failed");
		return 0;
	}*/
	return 1;
}

//---------------------------------------------------------------------------
// SOCK::SSLCheckCert
//---------------------------------------------------------------------------
void SOCK_SSLCheckCert(T_SOCK *tSOCK)
{
	X509 *peer;
	char peer_CN[256];

	if(SSL_get_verify_result(tSOCK->ssl)!=X509_V_OK)
		printf("Certificate doesn't verify");

	/*Check the cert chain. The chain length
	  is automatically checked by OpenSSL when
	  we set the verify depth in the ctx */

	/*Check the common name*/
	peer=SSL_get_peer_certificate(tSOCK->ssl);
	X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, 256);
	//if(strcasecmp(peer_CN,host))
		//cfg_log("Common name doesn't match host name");
}

//---------------------------------------------------------------------------
// SOCK::send
//		param
//			0	-	Simple lookup
//			1	-   GET Request
//			2	-	POST Request
//---------------------------------------------------------------------------
int SOCK_send(T_SOCK *tSOCK, int param, char buf[1024])
{
	static char *REQUEST_TEMPLATE=
	"GET %s HTTP/1.0\r\nUser-Agent:"
	"sumppump (+https://github.com/yagmoth555/sumppump)\r\nHost: %s:%d\r\n\r\n";

	char *request;
	char *p = NULL;

	int r;
	int request_len;
	unsigned long long	i = 0;
	unsigned long	temp = 0;
	//unsigned long	read_blocked_on_write=0;
	unsigned long	read_blocked=0;
	_Bool retry = 0;

	//if (!IsReady())

	// Reset global var
	ZeroMemory(tSOCK->cCode, G_INIMINPAGESIZE);
	tSOCK->lSize = 0;

	if (param==0) {
		request_len=strlen(REQUEST_TEMPLATE) + strlen(tSOCK->cHost) + strlen(tSOCK->cURI)+6;
	}
	else if (param==1) {
		request_len=strlen(buf);
	}
	/* Now construct our HTTP request */
	if(!(request=(char *)malloc(request_len)))
		return 0;
	ZeroMemory(request, request_len);

	if (param==0) {
		snprintf(request, request_len, REQUEST_TEMPLATE, tSOCK->cURI, tSOCK->cHost, tSOCK->iPort);
		/* Find the exact request_len */
		request_len=strlen(request);
	}
	else if (param==1) {
		strncpy(request, buf, request_len);
		//request[request_len] = 0;
	}

	write_retry:
	if (!param)
		r=SSL_write(tSOCK->ssl,request,request_len);
	else
		r=r=SSL_write(tSOCK->ssl,buf,strlen(buf));
	switch(SSL_get_error(tSOCK->ssl,r)){
		case SSL_ERROR_NONE:
			if(request_len!=r) {
				printf("Incomplete write!");
				return 0;
			}
			break;
		case SSL_ERROR_WANT_READ:
			printf("SSL_ERROR_WANT_READ");
			goto write_retry;

		case SSL_ERROR_WANT_WRITE:
			printf("SSL_ERROR_WANT_WRITE");
			goto write_retry;

		case SSL_ERROR_WANT_CONNECT:
			printf("SSL_ERROR_WANT_CONNECT");
			if (SOCK_Connect(tSOCK)==0)
				return 1;
			break;

		case SSL_ERROR_SYSCALL:
			printf("SSL_ERROR_SYSCALL");
			return 0;

		default:
			printf("SSL write problem");
			return 0;
	}
	if (request)
		free(request);
	
	printf(ANSI_COLOR_YELLOW "* Fletching (%s%s)" ANSI_COLOR_RESET, tSOCK->cHost, tSOCK->cURI);
	SOCK_recv(tSOCK);
	return 1;
}
	
//---------------------------------------------------------------------------
// SOCK::recv
//---------------------------------------------------------------------------
int SOCK_recv(T_SOCK *tSOCK)
{
	char *p = NULL;
	char *my = NULL;
	char szBuffer[BUFSIZZ];
	char szContentLength[255];
	char szChunkSize[255];
	unsigned long ulContentLength = 0;
	unsigned long ulHeaderLength = 0;
	unsigned long ulChunkSize = 0;
	int r;
	unsigned long long	i = 0;
	unsigned long	temp = 0;
	_Bool header, chunked, gzipped;
	
	tSOCK->lSize = 0;
	ulContentLength = 0;
	ZeroMemory(szBuffer, BUFSIZZ);
	header=false;
	chunked=false;
	gzipped=false;

	//if (BIO_set_nbio(tSOCK->sbio, 1) != 1) {
    //        printf("12");
	//		return 0;
	//}

	do {
		if (p)
			free(p);
		p = SOCK_readLine(tSOCK);
		if (!p)
			return 0;
		r = strlen(p);

		////////////
		// CR-LF
		if (r<=2) {
			my = strstr(p, "\r\n");
			if (my) {
				if (header==false) {
					header=true;
					// Get Content-Lenght
					my = strstr(tSOCK->cCode, "Content-Length: ");
					if (my) {
						my = my + strlen("Content-Length: ");
						i = 0;
						while ((my) && my[0]!=13) {
							if ((my[0] >= 48) && (my[0] <= 57)) {
								szContentLength[i] = my[0];
							}
							else
								break;
							i++;
							my++;
						}
						szContentLength[i]=0;
						ulContentLength = atol(szContentLength);
						printf(ANSI_COLOR_YELLOW " %s bytes \n" ANSI_COLOR_RESET, szContentLength);
					}
					// Get if Chunked
					my = strstr(tSOCK->cCode, "Transfer-Encoding: chunked");
					if (my)
						chunked=true;
					else
						chunked=false;		
					// Get if body gzipped
					my = strstr(tSOCK->cCode, "Content-Encoding: gzip");
					if (my)
						gzipped=true;
					else
						gzipped=false;
					ulHeaderLength = tSOCK->lSize;
				}
				else if (chunked)
					return 1;
			}
		}
		
		tSOCK->lSize = tSOCK->lSize + r;
		if (tSOCK->lSize > G_INIMINPAGESIZE) {
			tSOCK->lSize = G_INIMINPAGESIZE;
			return 0; // *** transfert incomplete ***
		}
		if (tSOCK->lSize>=84736)
			tSOCK->lSize = tSOCK->lSize;
		if (((header) && (ulContentLength)) && ((tSOCK->lSize-ulHeaderLength) >= ulContentLength))
			return 1; // *** transfert complete ***
		
		if (chunked) {
			if (ulChunkSize) {
				if (r != ulChunkSize)
					return 0;  // *** transfert incomplete ***
				strcat(tSOCK->cCode, p);
				ulChunkSize = 0;
			}
			else {
					i = 0;
					my = p;
					while ((my) && my[0]>47 && my[0]<58) {
						if ((my[0] >= 48) && (my[0] <= 57)) {
							szChunkSize[i] = my[0];
						}
						else
							break;
						i++;
						my++;
					}
					szChunkSize[i]=0;
					ulChunkSize = atol(szChunkSize);
					if (ulChunkSize==0)
						return 1; // *** transfert complete ***
			}
		} 
		else
			strcat(tSOCK->cCode, p);

	} while (p);

	return 1;
}

//---------------------------------------------------------------------------
// SOCK::readLine
//---------------------------------------------------------------------------
char *SOCK_readLine(T_SOCK *tSOCK)
{
	char *p = NULL, *t = NULL;
	char buffer;
	int r = 0;
	int i = 0;
	int pending = 1;
	int iReadSize = 1024;
	int iTotalReceived = 0;

	while (true) {
		if (pending) {
			r=SSL_read(tSOCK->ssl,&buffer,1);
			if (r <= 0) {
				switch(SSL_get_error(tSOCK->ssl,r)){
				case SSL_ERROR_NONE:
					//printf("SSL_ERROR_NONE");
					return p;
					break;
				case SSL_ERROR_ZERO_RETURN:
					//printf("SSL_ERROR_ZERO_RETURN\n");
					return p;
					break;
				case SSL_ERROR_WANT_READ:
					//printf("SSL_ERROR_WANT_READ");
					return p;
					break;
				case SSL_ERROR_WANT_WRITE:
					//printf("SSL_ERROR_WANT_WRITE");
					return p;
					break;
				case SSL_ERROR_SYSCALL:
					//printf("SSL_ERROR_SYSCALL");
					return p;
					break;
				default:
					break;
				}
			}
			pending=0;
		}
		else 
			break;

		i++;
		t = (char*) realloc (p, i * sizeof(char));
		if (t!=NULL) {
			p=t;
			p[i-1]=buffer;
		}
		else {
			free(p);
			return NULL;
		}

		if ((buffer == '\n') || (buffer == '\0')) {
			i++;
			t = (char*) realloc (p, i * sizeof(char));
			if (t!=NULL) {
				p=t;
				p[i-1]=0;
			}
			else {
				free(p);
				return NULL;
			}
			return p;
		}
		pending = SSL_pending(tSOCK->ssl);
	}
	
	i++;
	t = (char*) realloc (p, i * sizeof(char));
	if (t!=NULL) {
		p=t;
		p[i-1]=0;	
	}
	else {
		free(p);
		return NULL;
	}
	return p;
}

//---------------------------------------------------------------------------
// SOCK::HTTPRequest
//---------------------------------------------------------------------------
int SOCK_HTTPGetStatus(T_SOCK *tSOCK)
{
	char			return_code[4] = "";
	int				iReturnCode = 0;
	char 			*p = NULL;

	p = tSOCK->cCode;
	if (tSOCK->lSize > 13)
		strncpy (return_code,(p+9),3);
	else
		strncpy (return_code,"1",3);
	return_code[3] = 0;
	iReturnCode = atoi(return_code);
	if ((iReturnCode==0) || (iReturnCode < 0) || (iReturnCode > 1000))
		iReturnCode = 1;
	if (iReturnCode == 401)
		tSOCK->Authentified = false;

	return iReturnCode;
}