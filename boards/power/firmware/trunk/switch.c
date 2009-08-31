#include "switch.h"
#include"device.h"
#include <signal.h>
#include "timed.h"
#include "power.h"
uint8_t button_pressed =0;

void switch_init(void)
{
	P1DIR &= 0x0f;

	/* init the button and its interrupts */
	P2DIR &= ~0x08;		/* pin 2.3 as input */
	P2IFG &= ~0x08;		/* check int flag clear */
	P2IES &= ~0x08;		/* int on rising edge */
	P2SEL &= ~0x08;		/* pin set as i/o */
	P2IE |= 0x08;			/* enable interrupt for that pin */
	button_pressed = 0;
}

uint8_t switch_get(void)
{
	return (P1IN & 0xf0)>>4;
}

interrupt (PORT2_VECTOR) port2_isr(void){
	uint8_t p2ifg_l;
	p2ifg_l = P2IFG; 	/* read only once just to be safe */
	if (p2ifg_l &  0x08){ /* p2.3 interrupt hit  */

		
		TACTL &= ~TAIE;


		TACCTL0 &= ~TAIE;
		TACCTL1 &= ~CCIE;
		TACCTL2 &= ~CCIE;
		//TACTL TACCTL0 = TACCTL1 = TACCTL2 = TACCR1 = TACCR2 = TACTL =0;
		
		user_enable();
		P1OUT |= 2;
		pwr_set_motor(1);
		button_pressed = 1;
		P2IFG &= ~0x08;	/* clear interrupt flag */
	}else
		P2IFG =0;	/* must have been another, that we dont care about */
}
