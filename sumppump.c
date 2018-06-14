// * sumppump/sumppump.c
// * Copyright (c) 2018 Philippe Levesque <EMail: yagmoth555@yahoo.com>
// * https://github.com/yagmoth555/sumppump
// * lorky.eastus.cloudapp.azure.com

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
pthread_t 		threadSOCKDIGComment;
MYSQL           mysql;
MYSQL_RES       *res;

int 			mysql_write(MYSQL mysql, char *string);
static void 	print_depth_shift(int depth);
static void 	process_object(json_value* value, int depth);
static void 	process_array(json_value* value, int depth);
static void 	process_value(json_value* value, int depth);
int 			SP_dbOpen ();
int 			SP_dbTableStructure();
void 			*SP_DIG_GetComment (void *ptr);
void 			*SP_DIG_GetQuestion (void *ptr);
void 			*SP_SEAPI_GetComment (void *ptr);

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
	if (!(SP_dbOpen()))
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

	iRet = pthread_create( &threadSOCKDIGComment, NULL, SP_DIG_GetComment, NULL); 
	if(iRet) { 
        printf(ANSI_COLOR_CYAN	"Thread error\n"	ANSI_COLOR_RESET); 
        exit(EXIT_FAILURE); 
    } 
	pthread_join(threadSOCKDIGComment, NULL); 
	
	mysql_close(&mysql);
	printf("\nClosing\n");
	
	return 0;
}
 
//-------------------------------------------------------------------
void *SP_DIG_GetComment (void *ptr) {
	// MSE HNQ: https://stackexchange.com/?pagesize=50
	T_SOCK tSOCK_DIGComment;
	json_char* json; 
	json_value* value; 
	tSOCK_DIG.iThrottle = 300; // default value without an token
	tSOCK_DIG.iPort = 443;
	char *p1, *p2, *p3, *p4, *p5, *p6;
	char szBuffer[1024];
	char question_link[1024];
	char question_id[1024];
	char site[1024];
	MYSQL_ROW row;
	MYSQL_RES *res;
	
	printf(ANSI_COLOR_GREEN	"SE HNQ DIG Comments Thread Started\n"	ANSI_COLOR_RESET); 
	if (mysql_query(&mysql,"SELECT site, visit, link FROM question WHERE visit is NULL"))
		return 0;
	if (!(res = mysql_store_result(&mysql)))
		return 0;
	
	while ((row = mysql_fetch_row(res))) {
		strcpy(tSOCK_DIGComment.cHost, row[0]);
		strcpy(tSOCK_DIGComment.cURI, row[2]);

		SOCK_init(&tSOCK_DIGComment);
		SOCK_Connect(&tSOCK_DIGComment);
		SOCK_send(&tSOCK_DIGComment, 0, NULL);  

		// p1,p2 class="comment js-comment " data-comment-id="  | "> COMMENT-ID
		// <li id="comment-120663" class="comment js-comment " data-comment-id="120663"> | </li>
		
		
		/*p1 = strstr(tSOCK_DIG.cCode, "<div data-sid=\"");
			
		while (p1) {
			p2 = strstr(p1, "\" class=\"question-container\">");
			if (p2) { 
				snprintf(site, 1024, "%.*s", p2 - p1 - 15, p1 + 15);
				p3 = strstr(p2, "<a href=\"");
				if (p3) { 
					p4 = strstr(p3, "\">");
					if (p4) {
						snprintf(question_link, 1024, "%.*s", p4 - p3 - 9, p3 + 9);
						p3 = strstr(p3, "questions/");
						p3 += 10;
						p5 = strstr(p3, "/");
						if (p5) {
							snprintf(question_id, 1024, "%.*s", p5 - p3, p3);
							snprintf(szBuffer, 1024, "INSERT INTO question (question_id, site, link) VALUES ('%s', '%s', '%s')", question_id, site, question_link);
							mysql_write(mysql, szBuffer);	
						}
					}
				}
			}
			p1+=15;
			p1 = strstr(p1, "<div data-sid=\"");
		}*/
		snprintf(szBuffer, 1024, "UPDATE question SET question.visit=NOW() WHERE question.link='%s'", row[2]);
		printf("\n%s", szBuffer);
		//mysql_query(&mysql, szBuffer);
		//SOCK_delete(&tSOCK_DIGComment);
	}
	mysql_free_result(res);
}

//-------------------------------------------------------------------
void *SP_DIG_GetQuestion (void *ptr) {
	// MSE HNQ: https://stackexchange.com/?pagesize=50
	T_SOCK tSOCK_DIG;
	json_char* json; 
	json_value* value; 
	tSOCK_DIG.iThrottle = 300; // default value without an token
	tSOCK_DIG.iPort = 443;
	char *p1, *p2, *p3, *p4, *p5, *p6;
	char szBuffer[1024];
	char question_link[1024];
	char question_id[1024];
	char site[1024];
	
	strcpy(tSOCK_DIG.cHost, "stackexchange.com");
	strcpy(tSOCK_DIG.cURI, "/?pagesize=50");

	printf(ANSI_COLOR_GREEN	"SE HNQ DIG Question Thread Started\n"	ANSI_COLOR_RESET); 
	SOCK_init(&tSOCK_DIG);
	SOCK_Connect(&tSOCK_DIG);
	SOCK_send(&tSOCK_DIG, 0, NULL);  

	// p1,p2 = <div data-sid=" |" class="question-container">
	// p3,p4 = <a href=" | ">
	// p5,p6
	p1 = strstr(tSOCK_DIG.cCode, "<div data-sid=\"");
		
	while (p1) {
        p2 = strstr(p1, "\" class=\"question-container\">");
        if (p2) { 
			snprintf(site, 1024, "%.*s", p2 - p1 - 15, p1 + 15);
			p3 = strstr(p2, "<a href=\"");
			if (p3) { 
				p4 = strstr(p3, "\">");
				if (p4) {
					snprintf(question_link, 1024, "%.*s", p4 - p3 - 9, p3 + 9);
					p3 = strstr(p3, "questions/");
					p3 += 10;
					p5 = strstr(p3, "/");
					if (p5) {
						snprintf(question_id, 1024, "%.*s", p5 - p3, p3);
						snprintf(szBuffer, 1024, "INSERT INTO question (question_id, site, link) VALUES ('%s', '%s', '%s')", question_id, site, question_link);
						mysql_write(mysql, szBuffer);	
					}
				}
			}
		}
		p1+=15;
		p1 = strstr(p1, "<div data-sid=\"");
    }	
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
	printf(ANSI_COLOR_GREEN "Connecting to the database\n" ANSI_COLOR_RESET);
	if (!(mysql_init(&mysql))) {
		printf(ANSI_COLOR_RED "MySQL error %s\n" ANSI_COLOR_RESET, mysql_error(&mysql));
		return 0;
	}
    if (!(mysql_real_connect(&mysql, G_szDBIP, G_szDBUSER ,G_szDBPASS, G_szDBINSTANCE, 3306, NULL, 0))) {
		printf(ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, mysql_error(&mysql));    
		return 0;
	}

    printf(ANSI_COLOR_GREEN "Locking the database\n" ANSI_COLOR_RESET);
    if (mysql_select_db(&mysql, G_szDBINSTANCE)) {
        printf(ANSI_COLOR_RED "MySQL error %s\n" ANSI_COLOR_RESET, mysql_error(&mysql));
		return 0;
	}
	return 1;
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
// mysql_write()
//-------------------------------------------------------------------
int mysql_write(MYSQL mysql, char *string) {
	char *zErrMsg = 0;
	int iError = 0;

	if (mysql_query(&mysql, string)) {
		//printf("SQL error: %s\n %s\n", mysql_error(&mysql), string);
		return 0;
	}
		
	return 1;
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
