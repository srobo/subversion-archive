/* Routines for speaking to the ADS5030.
   Speaks to it in 2-wire read-only mode. */
#ifndef __ADS_5030_H
#define __ADS_5030_H
#include <stdint.h>
#include "sensor.h"
#include "ads5030_state.h"

/* Populates the sensor_t structure,
 * including the function pointers.
 * Args:
 *  - sensor: The structure to populate.
 *  -  clk_n: The clk pin number on port 2 (P2.clk_n)
 *  -  dio_n: The dio pin number on port 2 (P2.dio_n)
 */
void ads_5030_init( sensor_t* sensor,
		    uint8_t clk_n, 
		    uint8_t dio_n );

/* Returns the angular position */
int32_t ads_5030_read( sensor_t* sensor );

#endif	/* __ADS_5030_H */
