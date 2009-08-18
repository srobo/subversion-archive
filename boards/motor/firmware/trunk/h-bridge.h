/*  Copyright (C) 2009 Robert Spanton
    Routines for controlling the h-bridges 

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
#ifndef __H_BRIDGE_H
#define __H_BRIDGE_H
#include <stdint.h>

/* States a motor can be in */
typedef enum
{
	M_OFF = 0,
	M_FORWARD,
	M_BACKWARD,
	M_BRAKE
} h_bridge_state_t;

/* Initialise the h-bridges */
void h_bridge_init( void );

void h_bridge_set( uint8_t channel, h_bridge_state_t state );

#endif	/* __H_BRIDGE_H */
