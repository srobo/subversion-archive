#ifndef __COMMON_FD_H
#define __COMMON_FD_H
#include <glib.h>

/* Set a file to be nonblocking */
gboolean fd_set_nonblocking( int fd );

#endif	/* __COMMON_FD_H */
