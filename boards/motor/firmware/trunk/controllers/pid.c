#include "pid.h"
#include "pid_state.h"
#include "../motor.h"
#include "../i2c_desc.h"

static const i2c_setting_t pid_settings[] = {
	I2C_DESC_SETTING( ST_I16, pid_state_t, kp ),
	I2C_DESC_SETTING( ST_I16, pid_state_t, ki ),
	I2C_DESC_SETTING( ST_I16, pid_state_t, kd )
};	

static int16_t pid_next( controller_t* con,
			 int32_t target,
			 int32_t val );

void pid_init( controller_t *con )
{
	pid_state_t *pid = &(con->state.pid);

	con->next = pid_next;

	pid->i = 0;
	pid->last_e = 0;

	pid->kp = 0;
	pid->ki = 0;
	pid->kd = 0;

	con->i2c_tbl = pid_settings;
	con->i2c_tblen = sizeof( pid_settings ) / sizeof( i2c_setting_t );
}

static int16_t pid_next( controller_t* con,
			 int32_t target,
			 int32_t val )
{
	pid_state_t *pid = &(con->state.pid);
	int32_t e = target - val;
	int32_t c;

	/* P */
	c = e * pid->kp;

	/* I: Avoid integrator windup:
	   Only include the I term when the output isn't saturated */
	if( c < MOTOR_MAX && c > MOTOR_MIN ) {
		pid->i += e;
		c += (pid->i * pid->ki) / 128;
	} else
		pid->i = 0;

	/* TODO: Move above I */
	/* D */
	c += (e - pid->last_e) * pid->kd;
	pid->last_e = e;

	return c;
}