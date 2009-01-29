#ifndef __XBEE_H
#define __XBEE_H
#include <stdint.h>

#define XBSLEEP (1<<6)
#define XBRESET (1<<2)
#define RTS (1<<4)
#define CTS (1<<5)

void xbee_init(void);

#define CTSOK (!(P5IN& CTS ) )      /*  ok to send data when this evaluates true */

#define startRTS P5OUT &= ~ RTS
#define stopRTS P5OUT |= RTS
#define RTS_state P5OUT & RTS

#define xbee_on() P5OUT |= XBRESET
#define xbee_off() P5OUT &= ~XBRESET
#define xbee_state (P5OUT & XBRESET)

#endif	/* __XBEE_H */
