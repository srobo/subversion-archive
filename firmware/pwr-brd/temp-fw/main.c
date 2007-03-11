#include <p18f4550.h>

long int i;

void main (void)
{
//rd0-4 switch
//rd5-7 leds
//re0 - big power motor fet thing
//re1 - fet control servo rail
//re2 - fet slug rail

	TRISD= 0x0f;
	PORTD = 0;//0b10100000;
	TRISE = 0;
	PORTE = 0b111; // turn all power rails on
	while ( 1 )
	{
		PORTD^= 0xFF;
		
	for (i=0;i<99999;i++);
	
	}//end while
			
}// end main
