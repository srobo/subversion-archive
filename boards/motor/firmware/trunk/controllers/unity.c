#include "unity.h"
#include <stdint.h>

static int16_t unity_next( controller_t* con, int32_t target, int32_t val );

void unity_init( controller_t *con )
{
	con->next = unity_next;
}

static int16_t unity_next( controller_t* con, int32_t target, int32_t val )
{
	/* Wow, what a complicated control loop this is. */
	return target;
}
