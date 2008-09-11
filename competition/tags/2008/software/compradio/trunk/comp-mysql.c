#define _GNU_SOURCE
#include "comp-mysql.h"
#include <mysql/mysql.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The mysql handle */
static MYSQL *db = NULL;

gboolean sr_match_info( uint16_t N, match_t* m )
{
	char* q = NULL;
	MYSQL_RES *res;
	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	unsigned int n_fields, i;
	assert( m != NULL );
	m->time = 0;
	for(i=0; i<4; i++)
		m->teams[i] = 0;

	asprintf(&q, "SELECT * FROM matches WHERE number = %hu LIMIT 1;", N );
	if( mysql_query( db, q ) != 0 ) {
		fprintf(stderr, "Failed to grab match %hhu info.", N );
		return FALSE;
	}
	free(q);
      
	res = mysql_store_result( db );

	if( mysql_num_rows( res ) == 0 ) {
		printf("Match %hhu not found\n", N);
		mysql_free_result(res);
		return FALSE;
	}

	n_fields = mysql_num_fields( res );
	fields = mysql_fetch_fields( res );
	while((row = mysql_fetch_row(res))) {
		for(i=0; i<n_fields; i++) {
			/* printf("%s = %s\n", fields[i].name, row[i]); */

			if( strcmp(fields[i].name,"time") == 0 )
				m->time = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"red")==0)
				m->teams[RED] = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"blue")==0)
				m->teams[BLUE] = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"green")==0)
				m->teams[GREEN] = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"yellow")==0)
				m->teams[YELLOW] = strtoul(row[i], NULL, 10);
		}
	}
	mysql_free_result(res);
	return TRUE;
}

void sr_mysql_init( void )
{
	db = mysql_init( NULL );
	assert(db != NULL);

	if( mysql_real_connect( db, 
				"127.0.0.1", 
				"comp",
				"lemmings",
				"comp",
				8000,
				NULL,
				0 ) == NULL ) {
		fprintf(stderr, "Couldn't connect to mysql: %s\n",
			mysql_error(db));
		exit(1);
	}
}

void strtoaddr( char* str, xb_addr_t* addr )
{
	sscanf(str, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
	       &addr->addr[0], &addr->addr[1], &addr->addr[2], &addr->addr[3],
	       &addr->addr[4], &addr->addr[5], &addr->addr[6], &addr->addr[7] );
}


gboolean sr_team_get_addr( uint16_t number, xb_addr_t* addr )
{
	char *q = NULL;
	MYSQL_RES *res;
	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	unsigned int n_fields, i;
	assert(addr != NULL);
	addr->type = XB_ADDR_64;

/* 	uint8_t addr[8]; */
	asprintf(&q, "SELECT * FROM radios WHERE teamNumber = %hhu LIMIT 1;",
		 number);
	if( mysql_query( db, q ) != 0 ) {
		fprintf(stderr, "Failed to get team %hhu radio address.", number );
		return FALSE;
	}
	free(q);

	res = mysql_store_result( db );

	if( mysql_num_rows( res ) == 0 ) {
		printf("WARNING: Team %hhu radio address not found\n", number);
		mysql_free_result(res);
		return FALSE;
	}

	n_fields = mysql_num_fields( res );
	fields = mysql_fetch_fields( res );
	while((row = mysql_fetch_row(res))) {
		for(i=0; i<n_fields; i++) {
			/* printf("%s = %s\n", fields[i].name, row[i]); */
			if( strcmp(fields[i].name,"address") == 0 )
				strtoaddr( row[i], addr );
		}
	}
	mysql_free_result(res);
	return TRUE;
}
