#include "isense.h"
#include"device.h"


void isense_init(void)
{
	/* Isense EN = p5.0 */
	/* Gsel = P5.1 */
	P5DIR |= 0x03;		/* as o/p */
	
	P5OUT |= 0X02;		/* Select higher Gain */
	P5OUT |= 0X01;		/* Enable i sence */
}
