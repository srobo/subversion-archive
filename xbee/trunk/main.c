#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "xbee.h"

int main( int argc, char** argv )
{
	xbee_t xb;
	int sp = -1;

	if( argc < 2 )
	{
		fprintf( stderr, "Not enough arguments\n" );
		return 1;
	}

	sp = open( argv[1], O_RDWR | O_NONBLOCK );
	if( sp < 0 )
	{
		fprintf( stderr, "Error: Failed to open serial port\n" );
		return 1; 
	}
	
	if ( xbee_init( &xb, sp ) )
		xbee_main( &xb );

	xbee_free( &xb );

	if( close( sp ) < 0 )
	{
		fprintf( stderr, "Error: Failed to close serial port\n" ); 
		return 2;
	}

	return 0;
}
