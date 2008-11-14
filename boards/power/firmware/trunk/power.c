#include "power.h"
#include"device.h"
#include"led.h"

static void delay(int16_t time);

void pwr_init(void)
{
  set_led(0);
  P5OUT |= 0x80; 		/* set sug boot hi before un-tristate, else slug boot */
  P5DIR |= 0x80;
  P2DIR |= 0x07;
  delay(250);			/* let power settle to help mode B */
  set_led(1);

  P2OUT |= 0x01;		/* power slug rail */
  delay(250);
  set_led(2);
  
  P5OUT &= ~0x80;		/* blip down -PRESS THE BUTTON*/
  set_led(4);
  delay(50);
  P5OUT |= 0x80;		/* NEVER PRESS THE BUTTON */
  set_led(3);
  delay(50);
  set_led(6);
}

void pwr_set_slug(uint8_t val)
{
  if (val == 0) 
    P2OUT |= 0x01;
  else
    P2OUT &= ~0x01;
}
uint8_t pwr_get_slug(void)
{
  return (P2IN &=0x01);
}

void pwr_set_servo(uint8_t val)
{
  if (val == 0) 
    P2OUT |= 0x02;
  else
    P2OUT &= ~0x02;
}

uint8_t pwr_get_servo(void)
{
  return (P2IN &=0x02)>>1;
}

void pwr_set_motor(uint8_t val)
{
  if (val == 0) 
    P2OUT |= 0x04;
  else
    P2OUT &= ~0x04;
}

uint8_t pwr_get_motor(void)
{
  return (P2IN &=0x04)>>2;
}



static void delay(int16_t time)
{
    int16_t sponge =0;
    int16_t startupdel=0;
    for (startupdel=0;startupdel<(time*100);startupdel++)
    {
        for(sponge=0;sponge<250;sponge++);	
    }

}
