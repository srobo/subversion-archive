/* Generic interface to a controller */
#ifndef __CONTROLLERS_H
#define __CONTROLLERS_H
#include <stdint.h>
#include "pid_state.h"
#include "../i2c_desc.h"

struct controller;
typedef struct controller controller_t;

struct controller {
	/* Get the next output value.
	 * Args:
	 *  - target: The target value.
	 *  -    val: The new sensor reading. */
	int16_t (*next)( controller_t* con, int32_t target, int32_t val );

	/* Settings table */
	const i2c_setting_t* i2c_tbl;
	uint8_t i2c_tblen;

	union {
		/* unity controller doesn't have any state */
		pid_state_t pid;
	} state;
};

typedef void (*controller_init_func) ( controller_t *con );
#define NUM_CONTROLLER_TYPES 2

extern const controller_init_func controller_inits[NUM_CONTROLLER_TYPES];

#endif	/* __CONTROLLERS_H */
