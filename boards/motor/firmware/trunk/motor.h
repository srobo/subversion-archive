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
/* This is the motor layer. 
   It sits on top of the PWM layer - handles feedback etc. */
#ifndef __MOTOR_H
#define __MOTOR_H
#include "pwm.h"

/* buffer for current sense of motor channels*/
uint8_t adc_channel;
uint16_t currents [2];

/* At the moment it just sets the pwm ratio equal to speed */

typedef pwm_ratio_t speed_t;

/* States a motor can be in */
typedef enum
{
	M_OFF = 0,
	M_FORWARD,
	M_BACKWARD,
	M_BRAKE
} motor_state_t;

/* Set the speed/state of a motor */
void motor_set( uint8_t channel, speed_t speed,  motor_state_t state );

/* Get the speed of a motor */
speed_t motor_get_speed( uint8_t channel );

/* Get the state of a motor */
motor_state_t motor_get_state( uint8_t channel );

/* Initialise the motors */
void motor_init( void );

#endif	/* __PWM_H */
