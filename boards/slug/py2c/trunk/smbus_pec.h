#ifndef __SMBUS_PEC_H
#define __SMBUS_PEC_H
#include <stdint.h>

#define SMBUS_POLY    (0x1070U << 3)

uint8_t crc8( uint8_t tempdata );

#endif	/* __SMBUS_PEC_H*/
