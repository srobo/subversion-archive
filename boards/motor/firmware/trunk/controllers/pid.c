#include "pid.h"
#include "pid_state.h"

static int16_t pid_next( controller_t* con,
			 int32_t target,
			 int32_t val );

void pid_init( controller_t *con )
{
	con->next = pid_next;
}

static int16_t pid_next( controller_t* con,
			 int32_t target,
			 int32_t val )
{
	int32_t v = target - val;

	v = v * 5;
	if( v > 328 )
		v = 328;
	else if( v < -328 )
		v = -328;

	return v;
}
