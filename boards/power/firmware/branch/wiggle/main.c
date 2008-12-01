#include"device.h"
#include<io.h>
#include"hwinit.h"
#include<signal.h>
#include"led.h"
#include <stdint.h>
#include"switch.h"
#include"power.h"
#include"i2c.h"

int dummy =0; 			/* dummy variable to fix gdb bug */


int main(void)
{

	init_cpu();
	led_init();
	switch_init();
	//pwr_init();
	//i2c_init();
	//eint();			/* enable global interrupts */
	
	P3DIR |= 10;		/* sda and scl as outputs */
	while(1)
	{
		P3OUT ^= 13;
		
		
	}
}


/* 00 0  */
/* 01 1  */
/* 10 1 */
/* 11 0 */
