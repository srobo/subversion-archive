#include "control.h"
#include "sensors/sensor.h"
#include "controllers/controllers.h"
#include <stdint.h>
#include "sensors/null.h"
#include "sensors/ads5030.h"
#include "controllers/unity.h"
#include "controllers/pid.h"
#include "motor.h"

typedef struct
{
	sensor_t sensor;
	controller_t controller;

	int32_t target;
} channel_t;

channel_t channels[2];

void control_init( void )
{
	uint8_t i;

	for( i=0; i<2; i++ ) {
		channel_t *c = channels + i;

		c->target = 120;

		if( i==1 ) {
			null_init( &c->sensor );
			unity_init( &c->controller );
		} else {
			ads_5030_init( &c->sensor, 0, 1 );
			pid_init( &c->controller );
		}
	}
}

void control_step( void )
{
	uint8_t i;

	for( i=0; i<2; i++ ) {
		channel_t *c = channels + i;
		int32_t r;
		int16_t o;

		/* Read the sensor */
		r = c->sensor.read( &c->sensor );

		/* Put sensor value through control loop */
		o = c->controller.next( &c->controller, 
					c->target,
					r );

		if( o > MOTOR_MAX )
			o = MOTOR_MAX;
		else if( o < MOTOR_MIN )
			o = MOTOR_MIN;

		/* Set the motor */
		/* TODO: Negation needs to be configurable. */
		motor_set_n( i, 0-o );
	}
}
