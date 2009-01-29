#include"device.h"
#include<io.h>
#include"hwinit.h"
#include<signal.h>
#include"led.h"
#include <stdint.h>
#include"switch.h"
#include"power.h"
#include"i2c.h"
#include"timer-a.h"
#include"usart.h"
#include "adc.h"
#include "isense.h"
#include "xbee.h"
#include "flash430/i2c-flash.h"
#include "flash430/flash.h"

int dummy =0; 			/* dummy variable to fix gdb bug */

int main(void)
{
	init_cpu();
	led_init();
	switch_init();
	pwr_init();
	i2c_init();
	adc_init();
	isense_init();
	usart_init();
	xbee_init();
	timera_init();
	flash_init();
	i2c_flash_init();
	eint();			/* enable global interrupts */

	while(1)
	{
		if( i2c_flash_received )
			flash_switchover();
 	}
}

