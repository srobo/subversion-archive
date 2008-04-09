#include <stdio.h>
#include <xbee-conn.h>
#include "teams.h"

int main( int argc, char** argv )
{
	g_type_init();

	teams_read( "./teams.xml" );


	return 0;
}
