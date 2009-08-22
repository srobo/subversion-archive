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
#include "h-bridge.h"

#define MOTOR_MAX PWM_MAX
#define MOTOR_MIN (0-PWM_MAX)

/* Set the power of the motor.
   power can be positive or negative for forwards and backwards. */
void motor_set_n( uint8_t channel, int16_t power );

/* Set the power/state of a motor */
void motor_set( uint8_t channel, pwm_ratio_t power, h_bridge_state_t state );

/* Get the power of a motor */
pwm_ratio_t motor_get_power( uint8_t channel );

/* Get the state of a motor */
h_bridge_state_t motor_get_state( uint8_t channel );

/* Initialise the motors */
void motor_init( void );

#endif	/* __PWM_H */
