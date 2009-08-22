#ifndef __PID_STATE_H
#define __PID_STATE_H
#include <stdint.h>

typedef struct
{
	/* The I */
	int32_t i;

	/* For D */
	int32_t last_e;

	/* PID constants */
	/* TODO: Investigate these being int8_t */
	int16_t kp;
	/* ki is divided by 100 after multiplying */
	int16_t ki;
	int16_t kd;
} pid_state_t;

#endif	/* __PID_STATE_H */
