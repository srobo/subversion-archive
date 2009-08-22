/* Generic interface to a controller */
#ifndef __CONTROLLERS_H
#define __CONTROLLERS_H
#include <stdint.h>
#include "pid_state.h"

struct controller;
typedef struct controller controller_t;

struct controller {
	/* Get the next output value.
	 * Args:
	 *  - target: The target value.
	 *  -    val: The new sensor reading. */
	int16_t (*next)( controller_t* con, int32_t target, int32_t val );

	union {
		/* unity controller doesn't have any state */
		pid_state_t pid;
	} state;
};

#endif	/* __CONTROLLERS_H */
