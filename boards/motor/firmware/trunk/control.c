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
#include "i2c.h"

typedef struct
{
	/* Whether the controller is enabled */
	bool enabled;

	uint8_t sensor_n;
	sensor_t sensor;

	uint8_t controller_n;
	controller_t controller;

	int32_t target;

	/* Speed incrementer */
	struct {
		bool enabled;

		/* How much to increment the position by */
		int16_t inc;
		/* How often to increment the position */
		uint16_t period;

		/* How long since last increment */
		uint16_t counter;
	} speed;

} channel_t;

channel_t channels[2];

#define control_get_bank(channel) ( channel? 6:3 )
#define controller_get_bank(channel) ( channel? 4:1 )
#define sensor_get_bank(channel) ( channel? 5:2 )

/* HACK ALERT -- Macro to work out which channel is being used */
/* Only call this in the controller config routines */
#define CONTROL_CUR_CHANNEL ( (cmd_n >> 4)==3?0:1 )
/* Only call this in the sensor config routines */
#define SENSOR_CUR_CHANNEL ( (cmd_n >> 4)==3?0:1 )

/* Change controller */
static void controller_switch( uint8_t channel, uint8_t n );
/* Change sensor */
static void sensor_switch( uint8_t channel, uint8_t n );

/** I2C Commands **/
/* Change controller */
static void control_i2cw_controller( uint8_t *buf );
static uint8_t control_i2cr_controller( uint8_t* buf);

/* Change sensor */
static void control_i2cw_sensor( uint8_t *buf );
static uint8_t control_i2cr_sensor( uint8_t *buf );

static const i2c_setting_t control_i2c_tbl[] = {
	/*** Variables ***/
	/* Enabled */
	I2C_DESC_SETTING( ST_BOOL, channel_t, enabled ),

	/* The target */
	I2C_DESC_SETTING( ST_I32, channel_t, target ),

	/** Speed **/
	I2C_DESC_SETTING( ST_BOOL, channel_t, speed.enabled ),
	I2C_DESC_SETTING( ST_I16, channel_t, speed.inc ),
	I2C_DESC_SETTING( ST_I16, channel_t, speed.period ),

	/*** Functions ***/
	/* Controller selection */
	I2C_DESC_FUNC( 1, control_i2cw_controller, control_i2cr_controller ),
	/* Sensor selection */
	I2C_DESC_FUNC( 1, control_i2cw_sensor, control_i2cr_sensor )
};

void control_init( void )
{
	uint8_t i;

	for( i=0; i<2; i++ ) {
		channel_t *c = channels + i;
		uint8_t e;

		c->target = 0;
		c->enabled = FALSE;
		c->speed.enabled = FALSE;
		c->speed.inc = 10;
		c->speed.period = 400;
		c->speed.counter = 0;

		/* Default to the boring controller/sensor */
		controller_switch( i, 0 );
		sensor_switch( i, 0 );
		       
		e = control_get_bank(i);

		i2c_banks[e].settings = control_i2c_tbl;
		i2c_banks[e].tblen = sizeof(control_i2c_tbl)/sizeof(i2c_setting_t);
		i2c_banks[e].base = &channels[i];
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

/* Change to a different controller */
static void control_i2cw_controller( uint8_t *buf )
{
	uint8_t n = buf[0];

	if( n > NUM_CONTROLLER_TYPES )
		/* Ignore invalid values */
		return;

	controller_switch( CONTROL_CUR_CHANNEL, n );
}

/* Readback on the controller being used */
static uint8_t control_i2cr_controller( uint8_t* buf )
{
	buf[0] = channels[CONTROL_CUR_CHANNEL].controller_n;

	return 1;
}

/* Change to a different sensor */
static void control_i2cw_sensor( uint8_t *buf )
{
	uint8_t n = buf[0];

	if( n > NUM_SENSOR_TYPES )
		/* Ignore invalid values */
		return;
	sensor_switch( SENSOR_CUR_CHANNEL, n);
}

/* Readback on the sensor being used */
static uint8_t control_i2cr_sensor( uint8_t *buf )
{
	buf[0] = channels[SENSOR_CUR_CHANNEL].sensor_n;

	return 1;
}

/* Change controller */
static void controller_switch( uint8_t channel, uint8_t n )
{
	/* TODO: Destroy old controller? */
	if( controller_inits[n] != NULL ) {
		controller_inits[n]( &channels[channel].controller );
		channels[channel].controller_n = n;

		/* Set up i2c */
		i2c_bank_entry_t *ben = &i2c_banks[ controller_get_bank(channel) ];
		struct controller *c = &channels[channel].controller;

		ben->settings = c->i2c_tbl;
		ben->tblen = c->i2c_tblen;
		ben->base = &c->state;
	}
}

/* Change sensor */
static void sensor_switch( uint8_t channel, uint8_t n )
{	
	/* TODO: Destroy old sensor? */
	if( sensor_inits[n] != NULL ) {
		sensor_inits[n]( &channels[channel].sensor );
		channels[channel].sensor_n = n;

		/* Set up i2c */
		i2c_bank_entry_t *ben = &i2c_banks[ sensor_get_bank(channel) ];
		struct sensor *s = &channels[channel].sensor;

		ben->settings = s->i2c_tbl;
		ben->tblen = s->i2c_tblen;
		ben->base = &s->state;
	}
}
