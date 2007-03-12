#include <p18f4550.h>

long int i;
void init(void);
void delay(int time);


//rc0 slug boot switch blip low to boot

//rd0-4 switch
//rd5-7 leds
//re0 - big power motor fet thing
//re1 - fet control servo rail
//re2 - fet slug rail

	


void main (void)
{
	init();
	
	delay(20);
	PORTCbits.RC0=0;
	PORTD=0b00100000;
	delay(20);
	PORTCbits.RC0=1;
	while ( 1 )
	{
		PORTD^= 0xFF;
		
		delay(20);
	
	}//end while
			
}// end main


void init(void)
{
	PORTC=0X01; // MUST BE BEFORE UNTRISTATING ELSE SLUG BOOT!!!
	TRISA=0XFF;
	TRISB=0XFF;
	TRISC=0XFE;
	TRISD= 0x0f;
	TRISE = 0;

	PORTD = 0;//0b10100000;
	PORTE = 0b111; // turn all power rails on

}

void delay(int time)
{
	int sponge =0;
	//good time about 20
	i=0;
	
	for (i=0;i<(time*100);i++)
	{
		for(sponge=0;sponge<250;sponge++);
	}

}
