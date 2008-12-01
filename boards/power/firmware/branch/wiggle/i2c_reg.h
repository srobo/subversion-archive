#ifndef __I2C_REG_H
#define __I2C_REG_H
#include "types.h"
//#include<stdint.h>

typedef struct
{
	/* Returns the size of the register */
	uint16_t (*size) ( void );

	/* Generate data to send to the master.
	   data is a 32-byte buffer to put the data in.
	   Returns the number of bytes placed in the buffer. */
	uint8_t (*read) ( uint8_t* data );

	/* Process received data of length len */
	void (*write) ( uint8_t* data, uint8_t len );
} reg_access_t;

#define I2C_NUM_COMMANDS 9

extern const reg_access_t dev_regs[];

#endif	/* __I2C_REG_H */
