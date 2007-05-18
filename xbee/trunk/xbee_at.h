#ifndef __XBEE_AT_H
#define __XBEE_AT_H
#include "xbee-module.h"

/* Puts the device into at_command_mode */
gboolean xbee_module_at_mode( XbeeModule* xb );

/* Checks that the device is still in at_command_mode */
gboolean xbee_module_get_at_mode( XbeeModule* xb );

/* Put the device into API mode */
gboolean xbee_module_set_api_mode( XbeeModule* xb );

/* Puts a string out on the serial line */
gboolean xbee_module_puts( XbeeModule* xb, char* buf );

#endif	/* __XBEE_AT_H */
