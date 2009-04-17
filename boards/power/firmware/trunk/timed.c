#include "led.h"
#include"device.h"
#include <stdint.h>
#include "timed.h"
#include "power.h"

uint8_t alive = 1;
static uint16_t safe_count =0;
uint8_t override=0;	/* button override */

void stayingalive(void)
{
	alive = 1;		/* set by slug on start of python code */
	return;
}

void alive_service(void){	/* called every 0.1s by ccpa2 */
	static uint16_t alive_count =0;
	/* dissabled as we now have more confidence in !modeb */
/* 	if (alive==0){ */
/* 		if (alive_count >= BOOT_TIME){ */
/* 			slug_boot(1);	/\* power cycle the slug *\/ */
/* 			alive_count =0; */
/* 		} */
/* 		else */
/* 			alive_count++; */
/* 	} */
}

void timer_override(void){
	override =1;		/* buton pressed so override the serial timout routine */
	pwr_set_motor(1);	/* enable user test mode */
}



void make_safe(void){
	if(override != 1){	/* not user test mode */
		safe_count =0;
		pwr_set_motor(1);
	}
}

void safe_service(void){	/* called every 0.1s by ccpa2 */
	if (override !=1){	/* not user test mode */
		if(safe_count>=SAFE_TIMEOUT){
			safe_count =0;
			pwr_set_motor(0);	
		}
		else
			safe_count++;
		
		return;
	}
}
