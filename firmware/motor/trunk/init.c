#include "common.h"
#include "init.h"

/* Initialise the GPIO ports */
void init_gpio( void );

void init( void )
{
	init_gpio();
	pwm_init();
	motor_init();
	i2c_init();

}

void init_gpio( void )
{
	/* Set all to outputs */
	P1DIR = P2DIR = P3DIR = P4DIR = 0xFF;

	/* Configure all as IO initially */
	P1SEL = P3SEL = P4SEL = 0;

	/*  */
	P1SEL |= 0xC;
	P2SEL |= 0xC;
}
