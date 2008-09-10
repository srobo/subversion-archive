/*   Copyright (C) 2007 Robert Spanton

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
#ifndef __I2C_H
#define __I2C_H

#define I2C_ADDRESS 0x12

/* The commands */
enum {
	/* Identify the device */
	M_IDENTIFY,
	/* Set the motor power/direction */
	M_CONF,

	/* Send the motor 1 setting to the master */
	M_GET0,
	/* Send the motor 2 setting to the master */
	M_GET1,

	/* Firmware version */
	M_FIRMWARE_VER,
	/* Firmware chunk reception, and next-address transmission */
	M_FIRMWARE_CHUNK,
	/* Firmware CRC transmission and confirmation */
	M_FIRMWARE_CRC,

	M_LAST_COMMAND
};

void i2c_init( void );

/* Reset the I2C device */
void i2c_reset( void );

#endif	/* __I2C_H */
