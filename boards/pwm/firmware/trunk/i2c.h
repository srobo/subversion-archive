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

#define I2C_ADDRESS 0x2E

/* The commands */
enum {
	M_IDENTIFY,
	M_CONF,
	M_LAST_COMMAND
};

void i2c_init( void );

/* Reset the I2C device */
void i2c_reset( void );

#endif	/* __I2C_H */
