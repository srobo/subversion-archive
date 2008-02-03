/**i2c for msp430
header file
**/

#ifndef I2C_HEADER
#define I2C_HEADER

#define ADDRESS 0x1E
#define IDENTIFIER 0x0001U

void initialise_i2c(void);

/* Returns the number of available bytes of data.
 * Returns 0 if called before an i2c session is complete. */
char available_i2c_data(void);

/* Returns a pointer to the first byte of i2c data. */
char * get_i2cData(void);

/* Enable the i2c peripheral. */
void enable_i2c(void);

/* The ISR for the USI */
void isr_usi (void);

#endif
