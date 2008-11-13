#include"device.h"
#include<io.h>


/* #define goff P5OUT |= 0x01 */
/* #define gon P5OUT &= ~0x1 */
/* #define roff P5OUT |= 0x04 */
/* #define ron P5OUT &= ~0x4 */
/* #define rtog P5OUT ^= 0x4 */
/* #define gtog P5OUT ^= 0x1 */

/* void wait(void)          //delay function */
/* { */
/*   volatile int i;        //declare i as volatile int */
/*   for(i=0;i<3200;i++);  //repeat 32000 times */
/* } */

void init_wd(void)
{
  /* No watchdog reset */
  WDTCTL = WDTPW + WDTHOLD;
}

void init_osc(void)
{

  /* oscilator */
  /* setup dco */
  DCOCTL = 0xe0;// max freq | max mod
  BCSCTL1 = 0xC7;
  BCSCTL2 = 0;


/*   /\* switch over to XT1 as per datasheet ??! *\/ */
/*   while(1) */
/*     { */
/*       IFG1 &= ~OFIFG; 		/\* magic witchcraft from datasheet *\/ */
/*       wait(); */
/*       //OUT = P5OUT^0x04;  */
/*       if((IFG1&OFIFG)==0) */
/* 	{ */
/* 	  break; */
/* 	} */
/*     } */
/*   BCSCTL2 = 0xC0; /\*  make system clock from XT1 *\/ */


}

void init_io(void)
{
  P1DIR = P2DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0;
  
}

