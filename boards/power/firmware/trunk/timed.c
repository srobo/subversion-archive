#include "led.h"
#include"device.h"
#include <stdint.h>
#include "timed.h"
#include "power.h"


static uint16_t safe_count =0;
static uint8_t pinged=0;	/* button override */
static uint8_t cutoff_state= STANDBY;

void user_enable(void){
	cutoff_state = USER;
}

void game_enable(void){
	if(cutoff_state != USER) /* so an errant rail on command doesnt disable user mode */
		cutoff_state = GAME;
}

void reset_cutoff(void){
	pinged=1;
}

void safe_service(void){	/* called every 0.1s by ccpa2 */
	if(pinged)
		safe_count =0;
	if(cutoff_state ==GAME)
	{
		if(safe_count>=SAFE_TIMEOUT)
		{
			pwr_set_motor(0);
			safe_count =0;
			return;
		}
		else{
			safe_count++;
			pwr_set_motor(1);
		}
	}
}
