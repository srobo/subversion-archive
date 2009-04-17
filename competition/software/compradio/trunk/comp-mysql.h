#ifndef __COMP_MYSQL_H
#define __COMP_MYSQL_H
#include <gtk/gtk.h>
#include <stdint.h>
#include <xbee-conn.h>
#include "comp-types.h"

/* Initialise the mysql connection */
void sr_mysql_init( void );

/* Get the teams playing in match N.
   Fills in the struct pointed to by m.
   Returns true if match exists.
 */
gboolean sr_match_info( uint16_t N, match_t* m );

/* Get a team's XBee address */
gboolean sr_team_get_addr( uint16_t number, xb_addr_t* addr );

#endif	/* __COMP_MYSQL_H */
