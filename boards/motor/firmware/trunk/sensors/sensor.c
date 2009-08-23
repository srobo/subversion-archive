#include "sensor.h"
#include "null.h"
#include "ads5030.h"

const sensor_init_func sensor_inits[] = 
{
	null_init,
	ads_5030_init
};
