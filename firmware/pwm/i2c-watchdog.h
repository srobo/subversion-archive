#ifndef __I2C_WATCHDOG_H
#define __I2C_WATCHDOG_H

/* To be called when a START condition occurs. */
void i2c_watchdog_start( void );

/* To be called when a STOP condition occurs. */
void i2c_watchdog_stop( void );

/* Resets the i2c peripheral if necessary. */
void i2c_watchdog_check( void );

#endif	/* __I2C_WATCHDOG_H */
