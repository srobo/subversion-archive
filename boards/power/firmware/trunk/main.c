#include"device.h"
#include<io.h>
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
#include "timed.h"

int dummy =0; 			/* dummy variable to fix gdb bug */

void init(void);
void init_cpu(void);

int main(void)
{
	/* Make sure that the slug rail is always on */
	P2DIR |= 1;
	P2OUT |= 1;

	init();

	while(1)
	{
		if( i2c_flash_received )
			flash_switchover();
 	}
}

void init(void)
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
	timed_init();
	timera_init();
	flash_init();
	i2c_flash_init();
	eint();			/* enable global interrupts */
}

void init_cpu(void)
{
	/* No watchdog reset */
	WDTCTL = WDTPW + WDTHOLD;

	/* oscillator */
	/* setup dco */
	DCOCTL = 0xe0;// max freq | max mod
	BCSCTL1 = XT2OFF | XTS | 0xC7 | RSEL2 | RSEL1 | RSEL0;
	BCSCTL2 = 0;

//#define XTAL
#ifdef XTAL
	/*  */
	BCSCTL1 =  RSEL2 | RSEL1 | RSEL0; /* enable xt2 */
	BCSCTL2 = SELS;		    /* smclk sourced from xt2 */
#endif

	P1DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0;
	P1OUT = P3OUT =  P4OUT =  P5OUT =  P6OUT = 0x00;

	/* Don't touch bit 0 of P2OUT */
	P2DIR &= 1;
	P2OUT &= 1;

	P3SEL |= 0x0A;
}
