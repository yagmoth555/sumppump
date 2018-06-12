// * sumppump/sumppump.c
// * Copyright (c) 2018 Philippe Levesque <EMail: yagmoth555@yahoo.com>
// * https://github.com/yagmoth555/sumppump

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <ctype.h>
#include "mysql/mysql.h"
#include "socket.h"
#include "json.h"
#include "global.h"
#include "secret.h"

pthread_t 		threadSOCKSEAPI;
pthread_t 		threadSOCKDIG;
MYSQL           mysql;
MYSQL_RES       *res;

void 	*SP_SEAPI_GetComment (void *ptr);
void 	*SP_DIG_GetQuestion (void *ptr);
int 	SP_dbOpen ();
int 	SP_dbTableStructure();
static void print_depth_shift(int depth);
static void process_object(json_value* value, int depth);
static void process_array(json_value* value, int depth);
static void process_value(json_value* value, int depth);

//-------------------------------------------------------------------
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
	
	// *******************************************
	iRet = pthread_create( &threadSOCKSEAPI, NULL, SP_SEAPI_GetComment, NULL); 
	if(iRet) { 
        printf(ANSI_COLOR_CYAN	"Thread error\n"	ANSI_COLOR_RESET); 
        exit(EXIT_FAILURE); 
    } 
	pthread_join(threadSOCKSEAPI, NULL);
    iRet = pthread_create( &threadSOCKDIG, NULL, SP_DIG_GetQuestion, NULL); 
	if(iRet) { 
        printf(ANSI_COLOR_CYAN	"Thread error\n"	ANSI_COLOR_RESET); 
        exit(EXIT_FAILURE); 
    } 
	pthread_join(threadSOCKDIG, NULL); 
	
	printf("\nClosing\n");
	return 0;
} 

//-------------------------------------------------------------------
void *SP_DIG_GetQuestion (void *ptr) {
	// MSE HNQ: https://stackexchange.com/?pagesize=50
	T_SOCK tSOCK_DIG;
	json_char* json; 
	json_value* value; 
	tSOCK_DIG.iThrottle = 300; // default value without an token
	tSOCK_DIG.iPort = 443;
	strcpy(tSOCK_DIG.cHost, "stackexchange.com");
	strcpy(tSOCK_DIG.cURI, "/?pagesize=50");

	printf(ANSI_COLOR_GREEN	"SE HNQ DIG Thread Started\n"	ANSI_COLOR_RESET); 
	SOCK_init(&tSOCK_DIG);
	SOCK_Connect(&tSOCK_DIG);
	SOCK_send(&tSOCK_DIG, 0, NULL);    
}

//-------------------------------------------------------------------
void *SP_SEAPI_GetComment (void *ptr) {
	T_SOCK tSOCK_SEAPI;
	json_char* json; 
	json_value* value; 
	tSOCK_SEAPI.iThrottle = 300; // default value without an token
	tSOCK_SEAPI.iPort = 443;
	strcpy(tSOCK_SEAPI.cHost, "api.stackexchange.com");
	strcpy(tSOCK_SEAPI.cURI, "/2.2/comments?order=desc&sort=creation&site=stackoverflow");

	printf(ANSI_COLOR_GREEN	"SE API Comments Thread Started\n"	ANSI_COLOR_RESET); 
	SOCK_init(&tSOCK_SEAPI);
	SOCK_Connect(&tSOCK_SEAPI);
	SOCK_send(&tSOCK_SEAPI, 0, NULL);
	json = (json_char*)tSOCK_SEAPI.cCode; 
	value = json_parse(json,tSOCK_SEAPI.lSize); 
	if (value) { 
		process_value(value, 0);  
		json_value_free(value);  
    } else
		printf(ANSI_COLOR_RED "Stack Exchange API JSON not recognized.\n" ANSI_COLOR_RESET);
    //printf("%s\n", tSOCK_SEAPI.cCode);     
}

//-------------------------------------------------------------------
int SP_dbOpen () {
	char *zErrMsg = 0;
	int rc;
	
	printf(ANSI_COLOR_GREEN "Connecting to the database\n" ANSI_COLOR_RESET);
	if (!(mysql_init(&mysql)))
		printf(ANSI_COLOR_RED "MySQL error %s\n" ANSI_COLOR_RESET, mysql_error(&mysql));
    if (!(mysql_real_connect(&mysql, G_szDBIP, G_szDBUSER ,G_szDBPASS, G_szDBINSTANCE, 3306, NULL, 0)))
		printf(ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, mysql_error(&mysql));    

    printf(ANSI_COLOR_GREEN "Locking the database\n" ANSI_COLOR_RESET);
    if (mysql_select_db(&mysql, G_szDBINSTANCE))
        printf(ANSI_COLOR_RED "MySQL error %s\n" ANSI_COLOR_RESET, mysql_error(&mysql));
}

//-------------------------------------------------------------------
int SP_dbTableStructure () {
	char *zErrMsg = 0;
	int rc;
	char *sql1, *sql2, *sql3, *sql4, *sql5;

	sql1 = "CREATE TABLE sumppump ("  \
			"user_id INT PRIMARY KEY     NOT NULL," \
			"has_more INT," \
			"quota_max INT," \
			"quota_remaining INT);";
   
   sql2 = "CREATE TABLE user ("  \
			"user_id INT PRIMARY KEY     NOT NULL," \
			"reputation INT," \
			"user_id INT," \
			"user_type TEXT," \
			"accept_rate INT," \
			"profile_image TEXT," \
			"display_name TEXT," \
			"link TEXT," \
			"site TEXT," \
			"name TEXT); ";

	sql3 = "CREATE TABLE comment ("  \
			"user_id INT PRIMARY KEY     NOT NULL," \
			"edited INT," \
			"score INT," \
			"post_id TEXT," \
			"comment_id INT," \
			"user_type TEXT," \
			"display_name TEXT," \
			"link TEXT," \
			"reply_to_user INT," \
			"site TEXT," \
			"name TEXT); ";
			
	sql4 = "CREATE TABLE question ("  \
			"question_id INT PRIMARY KEY     NOT NULL," \
			"visit TEXT," \
			"site TEXT," \
			"link TEXT);";
			
	sql5 = "CREATE TABLE site ("  \
			"site TEXT PRIMARY KEY     NOT NULL," \
			"visit TEXT," \
			"link TEXT);";
	 
	/*rc = sqlite3_exec(db, sql1, 0, 0, &zErrMsg);
	if( rc ) {
		printf(ANSI_COLOR_YELLOW "Query error %s\n" ANSI_COLOR_RESET, zErrMsg);
		sqlite3_free(zErrMsg);
	} else
		printf(ANSI_COLOR_GREEN "Table created successfully\n" ANSI_COLOR_RESET);
	rc = sqlite3_exec(db, sql2, 0, 0, &zErrMsg);
	if( rc ) {
		printf(ANSI_COLOR_YELLOW "Query error %s\n" ANSI_COLOR_RESET, zErrMsg);
		sqlite3_free(zErrMsg);
	} else
		printf(ANSI_COLOR_GREEN "Table created successfully\n" ANSI_COLOR_RESET);
	rc = sqlite3_exec(db, sql3, 0, 0, &zErrMsg);
	if( rc ) {
		printf(ANSI_COLOR_YELLOW "Query error %s\n" ANSI_COLOR_RESET, zErrMsg);
		sqlite3_free(zErrMsg);
	} else
		printf(ANSI_COLOR_GREEN "Table created successfully\n" ANSI_COLOR_RESET);
	rc = sqlite3_exec(db, sql4, 0, 0, &zErrMsg);
	if( rc ) {
		printf(ANSI_COLOR_YELLOW "Query error %s\n" ANSI_COLOR_RESET, zErrMsg);
		sqlite3_free(zErrMsg);
	} else
		printf(ANSI_COLOR_GREEN "Table created successfully\n" ANSI_COLOR_RESET);
	rc = sqlite3_exec(db, sql5, 0, 0, &zErrMsg);
	if( rc ) {
		printf(ANSI_COLOR_YELLOW "Query error %s\n" ANSI_COLOR_RESET, zErrMsg);
		sqlite3_free(zErrMsg);
	} else
		printf(ANSI_COLOR_GREEN "Table created successfully\n" ANSI_COLOR_RESET);*/

}

//-------------------------------------------------------------------
 static void process_object(json_value* value, int depth) 
 { 
         int length, x; 
         if (value == NULL) { 
                 return; 
         } 
         length = value->u.object.length; 
         for (x = 0; x < length; x++) { 
                 print_depth_shift(depth); 
                 printf("object[%d].name = %s\n", x, value->u.object.values[x].name); 
                 process_value(value->u.object.values[x].value, depth+1); 
         } 
 } 
 
//-------------------------------------------------------------------
static void print_depth_shift(int depth) 
 { 
         int j; 
         for (j=0; j < depth; j++) { 
                 printf(" "); 
         } 
 } 
 
//------------------------------------------------------------------- 
 static void process_array(json_value* value, int depth) 
 { 
         int length, x; 
         if (value == NULL) { 
                 return; 
         } 
         length = value->u.array.length; 
         printf("array\n"); 
         for (x = 0; x < length; x++) { 
                 process_value(value->u.array.values[x], depth); 
         } 
 } 
 
//------------------------------------------------------------------- 
 static void process_value(json_value* value, int depth) 
 { 
         int j; 
         if (value == NULL) { 
                 return; 
         } 
         if (value->type != json_object) { 
                 print_depth_shift(depth); 
         } 
        switch (value->type) { 
                 case json_none: 
                        printf("none\n"); 
                        break; 
               case json_object: 
                        process_object(value, depth+1); 
                        break; 
                case json_array: 
                       process_array(value, depth+1); 
                        break; 
                 case json_integer: 
                         printf("int: %10" PRId64 "\n", value->u.integer); 
                         break; 
                 case json_double: 
                         printf("double: %f\n", value->u.dbl); 
                         break; 
                 case json_string: 
                         printf("string: %s\n", value->u.string.ptr); 
                         break; 
                case json_boolean: 
                         printf("bool: %d\n", value->u.boolean); 
                        break; 
         } 
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
