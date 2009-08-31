#include "led.h"
#include"device.h"
#include <stdint.h>
#include "timed.h"
#include "power.h"

static uint16_t safe_count;
//static uint8_t pinged;	/* button override */
static uint8_t cutoff_state;

void timed_init(void)
{
	safe_count = 0;

	cutoff_state = STANDBY;
}

void user_enable(void){
	cutoff_state = USER;
}

void game_enable(void){
	//if(cutoff_state != USER) /* so an errant rail on command doesnt disable user mode */
		cutoff_state = GAME;
	safe_count = 0;
}

void reset_cutoff(void){	/* called from a ping */
	safe_count =0;
	if (cutoff_state == GAME)
		pwr_set_motor(1);
}

void safe_service(void){	/* called every 0.1s by ccpa2 */	
	if (cutoff_state ==GAME)
	{
		if(safe_count>=SAFE_TIMEOUT)
		{
			pwr_set_motor(0);
			safe_count =0;
			return;
		}
		else{
			safe_count++;
		}
	}
}
