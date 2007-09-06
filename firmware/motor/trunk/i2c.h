#ifndef __I2C_H
#define __I2C_H

#define I2C_ADDRESS 0x12

/* The commands */
enum {
	M_IDENTIFY,
	M_CONF,
	M_LAST_COMMAND
};

void i2c_init( void );

#endif	/* __I2C_H */
