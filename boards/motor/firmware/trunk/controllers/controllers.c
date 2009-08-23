#include "controllers.h"
#include "unity.h"
#include "pid.h"

const controller_init_func controller_inits[NUM_CONTROLLER_TYPES] =
{
	unity_init,
	pid_init
};
