/*   Copyright (C) 2008 Robert Spanton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
#ifndef __I2C_FLASH
#define __I2C_FLASH
#include <stdint.h>
#include "common.h"

void i2c_flash_init( void );

/* Transmits the firmware version to the master. */
uint8_t i2c_flashr_fw_ver( uint8_t* buf );

/* Receives a chunk of firmware from the master.
   20 bytes expected. */
void i2c_flashw_fw_chunk( uint8_t* buf );

/* Transmits the address of the next required chunk to the master  */
uint8_t i2c_flashr_fw_next( uint8_t* buf );

/* Transmits the firmware CRC to the master */
uint8_t i2c_flashr_crc( uint8_t* buf );

/* Receives confirmation from the master that all 
   firmware has been received. 
   4-byte password expected. */
void i2c_flashw_confirm( uint8_t* buf );

/* When the firmware has been received and verified, this becomes TRUE */
extern bool i2c_flash_received;

#endif	/* __I2C_FLASH */
