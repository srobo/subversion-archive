/* Generic interface to a sensor */
#ifndef __SENSOR_H
#define __SENSOR_H
#include <stdint.h>

struct sensor;
typedef struct sensor sensor_t;

struct sensor {
	/* Read the state of the sensor */
	int32_t (*read)( sensor_t* self );

	union {
		/* null sensor has no state */
	} state;
};

#endif	/* __SENSOR_H */
