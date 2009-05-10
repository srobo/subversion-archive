#ifndef __I2C_FUNCS
#define __I2C_FUNCS
#include <stdint.h>

enum 
{
	/* Read the poll byte.
	   Format:
	      7: Write - i2c device has data to send
	      6: Read - i2c device waiting for data from master
	      5: Unused
	    4-0: Subsector Number (subsector = 32-byte chunk) */
	CMD_POLL = 2,

	/* Read the least significant 16-bits of the sector number */
	CMD_SECTOR_LOW = 11,
	/* Read the most significant 16-bits of the sector number */
	CMD_SECTOR_HIGH = 12,

	/* Read data from the i2c device*/
	CMD_RX_DATA = 8,

	/* Write data to the i2c device */
	CMD_TX_DATA = 9
};

typedef enum 
{
	MSD_READ,		/* I2C Device waiting for data */
	MSD_WRITE,		/* I2C Device has data */
	MSD_IDLE,
	MSD_ERROR
} msd_state_t;

/* Polls the i2c device state */
msd_state_t msd_poll( int fd );

/* Grabs all the information from poll */
msd_state_t msd_poll_full( int fd, uint8_t *subsector );

/* Get the sector number */
uint32_t msd_get_sector( int fd );

/* Send a 32 byte block to the i2c device */
void msd_send( int fd, uint8_t *data );

/* Read a 32 byte block from the i2c device */
void msd_recv( int fd, uint8_t *data );

#endif	/* __I2C_FUNCS */
