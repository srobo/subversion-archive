#include "device.h"
#include <io.h>

void init_cpu(void)
{
  /* No watchdog reset */
  WDTCTL = WDTPW + WDTHOLD;

  /* oscillator */
  /* setup dco */
  DCOCTL = 0xe0;// max freq | max mod
  BCSCTL1 = XT2OFF | XTS | 0xC7 | RSEL2 | RSEL1 | RSEL0;
  BCSCTL2 = 0;

#define XTAL
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

