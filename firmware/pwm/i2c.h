/**i2c for msp430
header file
**/

#ifndef I2C_HEADER
#define I2C_HEADER

/* The I2C Address - this is a 7-bit number */
#define ADDRESS 0x1E

/* The device identifier */
#define IDENTIFIER 0x0001U

/* I2C Commands */
enum
{
	/* Identify the device */
	COMMAND_IDENTIFY = 0,

	/* Set the servo position */
	COMMAND_SET,

	/* Read the servo positions back */
	COMMAND_READ
};

void initialise_i2c(void);

/* Enable the i2c peripheral. */
void enable_i2c(void);

/* The ISR for the USI */
void isr_usi (void);

#endif
