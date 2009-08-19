#include "null.h"

static int32_t null_read( sensor_t* self );

void null_init( sensor_t* sensor )
{
	sensor->read = null_read;
}

static int32_t null_read( sensor_t* self )
{
	/* OMG the insufferable complexity! */
	return 0;
}
