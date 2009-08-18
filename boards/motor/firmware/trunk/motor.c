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
#include "motor.h"

typedef struct 
{
	h_bridge_state_t state;
	speed_t speed;
} motor_t;

/* The motors */
motor_t motors[2];

/* Set the speed/state of a motor */
void motor_set( uint8_t channel, speed_t speed, h_bridge_state_t state )
{
	if( motors[channel].speed != speed )
	{
		pwm_set( channel, (pwm_ratio_t)speed );
		motors[channel].speed = speed;
	}

	/* Only set the state if it's changed */
	if( motors[channel].state != state )
	{
		h_bridge_set( channel, state );
		motors[channel].state = state;
	}

}

/* Get the speed of a motor */
speed_t motor_get_speed( uint8_t channel )
{
	return motors[channel].speed;
}

/* Get the state of a motor */
h_bridge_state_t motor_get_state( uint8_t channel )
{
	return motors[channel].state;
}

/* Initialise the motors */
void motor_init( void )
{
	uint8_t i;

	for( i=0; i<2; i++ )
	{
		/* Tease motor_set into actually making the changes we request */
		motors[i].state = M_BRAKE;
		motors[i].speed = 1;
		motor_set( i, 0, M_OFF );
	}
}
