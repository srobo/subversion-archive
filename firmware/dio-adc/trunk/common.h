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
