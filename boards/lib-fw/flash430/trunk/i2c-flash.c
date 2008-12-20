#include "i2c-flash.h"
#include "flash.h"

volatile bool i2c_flash_received;

void i2c_flash_init( void )
{
	i2c_flash_received = FALSE;
}

/* Transmits the firmware version to the master. */
uint8_t i2c_flashr_fw_ver( uint8_t* buf )
{
	/* Reinitialise the flashing system */
	flash_init();

	buf[1] = 0;
	buf[0] = 2;

	return 2;
}

/* Receives a chunk of firmware from the master.
   20 bytes expected. */
void i2c_flashw_fw_chunk( uint8_t* buf )
{
	uint16_t ver, addr;
	/* Format:
	    0-1: Firmware version (0 is lsb)
	    2-3: Address (2 is lsb)
	   4-19: The data */
	ver = buf[0] | (((uint16_t)buf[1]) << 8);
	addr = buf[2] | (((uint16_t)buf[3]) << 8);

	flash_rx_chunk( addr, (uint16_t*)(buf + 4) );
}

/* Transmits the address of the next required chunk to the master  */
uint8_t i2c_flashr_fw_next( uint8_t* buf )
{
	buf[0] = ((uint16_t)next_chunk) & 0xff;
	buf[1] = ( ((uint16_t)next_chunk) >> 8 ) & 0xff;

	return 2;
}

/* Transmits the firmware CRC to the master */
uint8_t i2c_flashr_crc( uint8_t* buf )
{
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	return 4;
}

/* Receives confirmation from the master that all 
   firmware has been received. 
   4-byte password expected. */
void i2c_flashw_confirm( uint8_t* buf )
{
	/* TODO: Do the CRC */
	i2c_flash_received = TRUE;
}
