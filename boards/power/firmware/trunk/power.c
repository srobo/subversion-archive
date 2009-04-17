#include "power.h"
#include "device.h"
#include "led.h"
#include "timed.h"
void delay(int16_t time);

void pwr_init(void)
{
	P2DIR |= 0x07;		/* power rail enable control pins as out */
	P2OUT &= ~0x07;
	slug_boot(0);
}

void slug_boot(uint8_t reboot){

	if (reboot)
		P2OUT &= ~0x01;	/* reeboot so power off first */
   
	delay(250);			/* let power settle to help mode B */
	P2OUT |= 0x01;		/* power slug rail */
	delay(250);

	P5OUT &= ~0x80;	   /* slugboot low then untristate */
	P5DIR |= 0x80;		/* slugboot set as output */	
	delay(50);

	delay(50);

	P5DIR &= ~0x80;		/* retristate the pin */
}

void pwr_set_slug(uint8_t val)
{
	if (val == 0) 
		P2OUT &= ~0x01;
	else
		P2OUT |= 0x01;

}
uint8_t pwr_get_slug(void)
{
	
	return (P2IN & 0x01);
}

void pwr_set_servo(uint8_t val)
{
	if (val == 0) 
		P2OUT &= ~0x02;
	else
		
		P2OUT |= 0x02;
}

uint8_t pwr_get_servo(void)
{
	return ((P2IN & 0x02)>>1);
}

void pwr_set_motor(uint8_t val)
{
  if (val == 0) 
	  P2OUT &= ~0x04;
  else{
	  game_enable();
	  pwr_set_servo(1);
	  P2OUT |= 0x04;
  }
}

uint8_t pwr_get_motor(void)
{
  return (P2IN & 0x04)>>2;
}

void delay(int16_t time)
{
    int16_t sponge =0;
    int16_t startupdel=0;

    for (startupdel=0;startupdel<(time*100);startupdel++)
    {
        for(sponge=0;sponge<250;sponge++);	
    }
}
