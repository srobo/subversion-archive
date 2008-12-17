#include "xbee.h"
#include"device.h"


void xbee_init(void)
{
	P5OUT &= ~XBSLEEP;
	P5DIR |= XBSLEEP;	
	
	P5OUT &= ~XBRESET;	/* active low so hold in reset*/
	P5DIR |= XBRESET;


}
