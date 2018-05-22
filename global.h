#ifndef   globalH
#define   globalH
//---------------------------------------------------------------------------
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <stdbool.h>
//---------------------------------------------------------------------------
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
//---------------------------------------------------------------------------
// SOCK
typedef struct t_SOCK {
	// html
	char *cHeader;
	char *cCode;
	unsigned long long lSize;
	// tcp
	char cHost[255];
	char cURI[255];
	int iPort;
	// ssl
	SSL *ssl;
	BIO *sbio;
	BIO *rbio;
	BIO *wbio;
	int sock;
	SSL_CTX *ctx;
	_Bool Authentified;
	_Bool Connected;
	unsigned long long status;
	char *pass;
	// stackexchange
	int iThrottle;
	struct t_SOCK  	*prev;
} T_SOCK, *PT_SOCK;

// USER
typedef struct t_USER {
	int reputation;
	int user_id;
	char user_type[255];
	int accept_rate;
	char profile_image[255];
	char display_name[255];	
	char link[255];	 
	char name[255];
} T_USER, *PT_USER;

// COMMENT
typedef struct t_COMMENT {
	int user_id;
	int edited;
	int score;
	int creation_date;
	int post_id;
	int comment_id;
	char user_type[255];
	int accept_rate;
	char profile_image[255];
	char display_name[255];	
	char link[255];	 
	char name[255];
	// extra : reply_to_user
	int user_id;
} T_COMMENT, *PT_COMMENT;

//---------------------------------------------------------------------------
extern char			*mystristr(char *haystack, const char *needle);
//---------------------------------------------------------------------------
extern unsigned int	g_uiExchPort;
extern char			g_cExchHost[255];
extern char			g_cExchFile[255];
extern char			g_cExchUsername[255];
extern char			g_cExchPassword[255];
extern char			g_cExchDomain[255];
extern char			g_cExchGAL[255];
extern char			g_cDBHost[255];
extern unsigned int	g_iDBPort;
extern char			g_cDBUsername[255];
extern char			g_cDBPassword[255];
extern char			g_cDBSchema[255];
extern char			g_cInstallPath[255];
extern char			g_cDBPath[255];
//---------------------------------------------------------------------------
#endif