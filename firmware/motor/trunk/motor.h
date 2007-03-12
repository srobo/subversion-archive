/* This is the motor layer. 
   It sits on top of the PWM layer - handles feedback etc. */
#ifndef __MOTOR_H
#define __MOTOR_H
#include "pwm.h"

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
