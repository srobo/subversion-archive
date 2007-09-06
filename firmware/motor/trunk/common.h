#include <msp430x22x4.h>

#define FLAG() P4OUT |= 0x80
#define FLAG_OFF() P4OUT &= ~0x80

/* These should be migrated to msp430 libc */
#define UCSSEL_UCLKI UCSSEL_0
#define UCSSEL_ACLK UCSSEL_1
#define UCSSEL_SMCLK UCSSEL_2

/* As should these */
#define UCMODE_SPI_3PIN UCMODE_0
#define UCMODE_SPI_4PIN_STE1 UCMODE_1
#define UCMODE_SPI_4PIN_STE0 UCMODE_2
#define UCMODE_I2C UCMODE_3
