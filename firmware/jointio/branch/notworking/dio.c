#include "common.h"
#include "dio.h"


void dio_init ( void ){
	P1DIR |= 0xF;		//set lower four pins to outputs
	P1SEL &= 0xF0;		//set lower four pins for I/O function
	P1OUT  &= 0xF0;		//clear ouput pins
	
}
