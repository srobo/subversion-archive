/* Monitor the 5V rail status */
#ifndef __RAILMON_H
#define __RAILMON_H
#include "common.h"

#define RAIL_MONITOR_PIN 0x10

/* Macro for determining whether the rail is up  */
#define railmon_is_up() ( P2IN & RAIL_MONITOR_PIN )

/* Initialise the rail monitoring situation  */
void railmon_init( void );

#endif	/* __RAILMON_H */
