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
 *  - sensor: The structure to populate. */
void ads_5030_init( sensor_t* sensor );

#endif	/* __ADS_5030_H */
