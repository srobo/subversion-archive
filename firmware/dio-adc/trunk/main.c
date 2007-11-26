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
#include "common.h"
#include "i2c.h"
#include "init.h"
#include <msp430/adc10.h>
#include "adc.h"

static int i = 0;


void UNEXPECTED()
{
}

int main( void )
{
	i = 0;
	
	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();
	adc_init();
	adc_sample();
	
	while(1)
	{
	}
}

