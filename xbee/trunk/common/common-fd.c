#include "common-fd.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

gboolean fd_set_nonblocking( int fd )
{
	int flags;

	flags = fcntl( fd, F_GETFL );
	if( flags == -1 )
	{
		fprintf( stderr, "Failed to set non-blocking IO: %m\n" );
		return FALSE;
	}

	flags |= O_NONBLOCK;

	if( fcntl( fd, F_SETFL, flags ) == -1 )
	{
		fprintf( stderr, "Failed to set non-blocking IO: %m\n" );
		return FALSE;
	}

	return TRUE;
}
