#include "led.h"
#include"device.h"
#include <stdint.h>
#include "timed.h"
#include "power.h"

uint8_t alive =0;
static uint16_t safe_count =0;
uint8_t override=0;	/* button override */


void stayingalive(void)
{
	alive = 1;		/* set by slug on start of python code */
	return;
}

void alive_service(void){	/* called ever 0.1s */
	static uint16_t alive_count =0;
	if (alive==0){
		if (alive_count >= BOOT_TIME){
			slug_boot(1);	/* power cycle the slug */
			alive_count =0;
		}
		else
			alive_count++;
	}
	
}

void button_override(void){
	override =1;		/* buton pressed so override the serial timout routine */
	pwr_set_motor(1);
}

void make_safe(void){
	if(override != 1){
		togc;
		safe_count =0;
		pwr_set_motor(1);
	}
}


void safe_service(void){
	if (override !=1){
		if(safe_count>=SAFE_TIMEOUT){
			safe_count =0;
			pwr_set_motor(0);	
		}
		else
			safe_count++;
		
		return;
	}
}
