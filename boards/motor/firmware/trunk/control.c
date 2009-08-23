#include "control.h"
#include "sensors/sensor.h"
#include "controllers/controllers.h"
#include <stdint.h>
#include "sensors/null.h"
#include "sensors/ads5030.h"
#include "controllers/unity.h"
#include "controllers/pid.h"
#include "motor.h"
#include "types/bool.h"

typedef struct
{
	/* Whether the controller is enabled */
	bool enabled;
	sensor_t sensor;
	controller_t controller;

	int32_t target;

	/* Speed incrementer */
	struct {
		bool enabled;

		/* How much to increment the position by */
		uint8_t inc;
		/* How often to increment the position */
		uint16_t period;

		/* How long since last increment */
		uint16_t counter;
	} speed;

} channel_t;

channel_t channels[2];

void control_init( void )
{
	uint8_t i;

	for( i=0; i<2; i++ ) {
		channel_t *c = channels + i;

		c->target = 2;
		c->enabled = FALSE;
		c->speed.enabled = FALSE;
		c->speed.inc = 32;
		c->speed.period = 1000;
		c->speed.counter = 0;

		if( i==1 ) {
			null_init( &c->sensor );
			unity_init( &c->controller );
		} else {
			ads_5030_init( &c->sensor, 0, 1 );
			pid_init( &c->controller );
			c->speed.enabled = TRUE;
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

		if( !c->enabled ) {
			motor_set_n( i, 0 );
			continue;
		}

		if( c->speed.enabled ) {
			c->speed.counter++;
			if( c->speed.counter > c->speed.period ) {
				c->target += c->speed.inc;
				c->speed.counter = 0;
			}
		}

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
		motor_set_n( i, o );
	}
}
