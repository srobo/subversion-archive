#ifndef __XBEE_INCOMING_H
#define __XBEE_INCOMING_H
#define _GNU_SOURCE
#include <glib.h>

#include "xbee.h"

/*** Incoming Data Functions ***/

/* Process incoming data */
gboolean xbee_proc_incoming( xbee_t* xb );

/* Reads in available bytes from the input.
 * When a full frame is achieved, it returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
int xbee_read_frame( xbee_t* xb  );


#endif	/* __XBEE_INCOMING_H */
