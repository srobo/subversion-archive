#include "device.h"
#include <io.h>

/* void wait(void)          //delay function */
/* { */
/*   volatile int i;        //declare i as volatile int */
/*   for(i=0;i<3200;i++);  //repeat 32000 times */
/* } */

void init_cpu(void)
{
  /* No watchdog reset */
  WDTCTL = WDTPW + WDTHOLD;

  /* oscillator */
  /* setup dco */
  DCOCTL = 0xe0;// max freq | max mod
  BCSCTL1 = 0xC7;
  BCSCTL2 = 0;

  P1DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0;
  P2DIR &= 0x02;			/* try and keep slug alive */
	P3SEL |= 0x0A;
  
}

