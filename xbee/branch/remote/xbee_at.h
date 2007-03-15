#ifndef __XBEE_AT_H
#define __XBEE_AT_H
#include "xbee.h"

/* Puts the device into at_command_mode */
gboolean xbee_at_mode( xbee_t* xb );

/* Checks that the device is still in at_command_mode */
gboolean xbee_get_at_mode( xbee_t* xb );

/* Put the device into API mode */
gboolean xbee_set_api_mode( xbee_t* xb );

/* Puts a string out on the serial line */
gboolean xbee_puts( xbee_t* xb, char* buf );

#endif	/* __XBEE_AT_H */
