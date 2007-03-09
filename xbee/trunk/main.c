#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "xbee.h"

int main( int argc, char** argv )
{
	xbee_t xb;
	int sp = -1;

	sp = open( "/dev/ttyUSB0", O_RDWR | O_NONBLOCK );
	if( sp < 0 )
	{
		fprintf( stderr, "Error: Failed to open serial port\n" );
		return 1; 
	}
	
	xbee_init( &xb, sp );

	xbee_main( &xb );

	if( close( sp ) < 0 )
	{
		fprintf( stderr, "Error: Failed to close serial port\n" ); 
		return 2;
	}

	return 0;
}
