#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <ctype.h>
#include "socket.h"
#include "sqlite3.h"
#include "json.h"
#include "global.h"

pthread_t 		threadSOCKSEAPI;
sqlite3 *db;

void 	*SP_GetComment (void *ptr);
int 	SP_dbOpen ();
int 	SP_dbTableStructure();


int main(void) { 
	int iRet = 0;
	printf(ANSI_COLOR_CYAN	"sumppump 0.0 - by yagmoth555\n"	ANSI_COLOR_RESET);
	printf(ANSI_COLOR_MAGENTA	"*******************************************************************\n");
	printf("                  {}\n");  
	printf("  ,   A           {}\n");  
	printf(" / \\, | ,        .--.\n");  
	printf("|  =|= >        /.--.\\\n");  
	printf(" \\ /` | `       |====|\n");  
	printf("  `   |         |`::`|\n");  
	printf("      |     .-;`\\..../`;_.-^-._\n");  
	printf("     /\\\\/  /  |...::..|`   :   `|\n");  
	printf("     |:'\\ |   /'''::''|   .:.   |\n");  
	printf("\n");  
	printf(".:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:._.:*~*:\n"	ANSI_COLOR_RESET);  
    // *******************************************
	if (SP_dbOpen())
		return 0;
	SP_dbTableStructure();
	
	// Get Comment by the StackExchange API
	iRet = pthread_create( &threadSOCKSEAPI, NULL, SP_GetComment, NULL); 
    if(iRet) { 
        printf(ANSI_COLOR_CYAN	"Thread error\n"	ANSI_COLOR_RESET); 
        exit(EXIT_FAILURE); 
    } 
    

	pthread_join(threadSOCKSEAPI, NULL); 
	printf("\nClosing");
	return 0;
} 


//-------------------------------------------------------------------
void *SP_GetComment (void *ptr) {
	T_SOCK tSOCK_SEAPI;
	tSOCK_SEAPI.iThrottle = 300; // default value without an token
	tSOCK_SEAPI.iPort = 443;
	strcpy(tSOCK_SEAPI.cHost, "api.stackexchange.com");
	strcpy(tSOCK_SEAPI.cURI, "/2.2/comments?order=desc&sort=creation&site=stackoverflow");
	// https://api.stackexchange.com/2.2/comments?order=desc&sort=creation&site=stackoverflow
	
	printf(ANSI_COLOR_GREEN	"Comments Thread Started\n"	ANSI_COLOR_RESET); 
	SOCK_init(&tSOCK_SEAPI);
	SOCK_Connect(&tSOCK_SEAPI);
	SOCK_send(&tSOCK_SEAPI, 0, NULL);
}

//-------------------------------------------------------------------
int SP_dbOpen () {
   char *zErrMsg = 0;
   int rc;

   rc = sqlite3_open("database.db", &db);
   if( rc ) {
      printf(ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(db));
      return 1;
   } else {
      printf(ANSI_COLOR_GREEN "Opened database successfully\n" ANSI_COLOR_RESET);
	  return 0;
   }
}

//-------------------------------------------------------------------
int SP_dbTableStructure () {
	char *zErrMsg = 0;
	int rc;
	char *sql;

	sql = "CREATE TABLE sumppump ("  \
         "user_id INT PRIMARY KEY     NOT NULL," \
		/*"has_more": true,
		"quota_max": 10000,
		"quota_remaining": 9995*/
   
   sql = "CREATE TABLE user ("  \
         "user_id INT PRIMARY KEY     NOT NULL," \
		 /*
		"reputation": 216,
        "user_id": 4439019,
        "user_type": "registered",
        "accept_rate": 91,
        "profile_image": "https://www.gravatar.com/avatar/ef8889f7e466495157e2527a85930a9c?s=128&d=identicon&r=PG&f=1",
        "display_name": "JD2775",
        "link"*/	 
         "name           TEXT    NOT NULL ); ";

	sql = "CREATE TABLE comment ("  \
         "user_id INT PRIMARY KEY     NOT NULL," \
		/*
		"edited": false,
		"score": 0,
		"creation_date": 1526674011,
		"post_id": 50418549,
		"comment_id": 87852650
		*/	 
		/*
		"reply_to_user": {
        "reputation": 14757,
        "user_id": 1790644,
        "user_type": "registered",
        "accept_rate": 80,
        "profile_image": "https://www.gravatar.com/avatar/f1a2a40ddbc52508d5b53e9879961251?s=128&d=identicon&r=PG",
        "display_name": "Matt Clark",
        "link": "https://stackoverflow.com/users/1790644/matt-clark"
		*/
         "name           TEXT    NOT NULL ); ";
	 
   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    
   if( rc ) {
      printf(ANSI_COLOR_YELLOW "Query error %s\n" ANSI_COLOR_RESET, zErrMsg);
	  sqlite3_free(zErrMsg);
   } else
      printf(ANSI_COLOR_GREEN "Table created successfully\n" ANSI_COLOR_RESET);
}

//-------------------------------------------------------------------
char *mystristr(char *haystack, const char *needle)
{
	const char *h, *n;

	if ( !*needle )
		return haystack;
		
	/*if ( !*haystack )
		return haystack;*/

	for ( ; *haystack; ++haystack ) {
		if (toupper(*haystack) == toupper(*needle)) {
			for (h = haystack, n = needle; *h && *n; ++h, ++n) {
				if (toupper(*h) != toupper(*n))
					break;
			}
			if (!*n)
				return haystack;
		}
	}
	return 0;
}
