/* Generic interface to a sensor */
#ifndef __SENSOR_H
#define __SENSOR_H
#include <stdint.h>
#include "ads5030_state.h"
#include "../i2c_desc.h"

struct sensor;
typedef struct sensor sensor_t;

struct sensor {
	/* Read the state of the sensor */
	int32_t (*read)( sensor_t* self );

	/* Settings table */
	const i2c_setting_t* i2c_tbl;
	uint8_t i2c_tblen;

	union {
		/* null sensor has no state */
		ads_5030_state_t ads5030;
	} state;
};

typedef void (*sensor_init_func) (sensor_t*);
#define NUM_SENSOR_TYPES 2

/* Table of sensor initialisation functions */
extern const sensor_init_func sensor_inits[NUM_SENSOR_TYPES];

#endif	/* __SENSOR_H */
