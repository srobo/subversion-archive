#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "xbee.h"

int main( int argc, char** argv )
{
	xbee_t *xb;

	if( argc < 2 )
	{
		fprintf( stderr, "Not enough arguments\n" );
		return 1;
	}

	xb = xbee_open( argv[1] );

	xbee_main( xb );

	xbee_close( xb );

	return 0;
}
