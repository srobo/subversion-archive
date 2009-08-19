/* Generic interface to a sensor */
#ifndef __SENSOR_H
#define __SENSOR_H
#include <stdint.h>
#include "ads5030_state.h"

struct sensor;
typedef struct sensor sensor_t;

struct sensor {
	/* Read the state of the sensor */
	int32_t (*read)( sensor_t* self );

	union {
		/* null sensor has no state */
		ads_5030_state_t ads5030;
	} state;
};

#endif	/* __SENSOR_H */
