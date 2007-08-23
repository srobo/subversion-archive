/*
-=Student Robotics=-
05/04/07: A Johnson

This firmware enables a MSP430F2012 to control up to 6 independant servos and
recieve or transmit data via i2c.
Servo are controled via a PWM signal with a pulse width between 1-2ms, but
experimentally it is found that maximum arm rotation (~100degrees) requires
pulses between 0.8-2.3ms.
This code assumes that the clock is running at  12Mhz
CC1 interrupts at the end of every pulse width (0.8-2.3ms) and is clocked from
timerA.
CC0 is the period of each pulse and interrupts at 20ms.


TODO-update all comments so they reflect what is happening
TODO-change all times so they are not magic values
TODO-Move all of i2c code out of interrupts, change them so they are polled
TODO-Get setServoPWM to return success or failure
*/

#include "hardware.h"
#include <stdint.h>
#include "i2c.h"
#include <signal.h>

uint16_t servo_pulse[SERVO_NUMBER]; //contains the pulse width of all servos
uint8_t current_servo;//the number of the current servo, between 0-SERVO_NUMBER


/**
Delay function, will run a while loop corresponding to 1ms when FOSC is 12Mhz
@param uint32 number of ms the delay must last
TODO -  possibly investigate a bug that increments the duration of the delay
inexplicably when a value greater than 30000 is given to i.
*/
void delay_MS(uint32_t d) {
	volatile uint16_t i;
	while(d--){
		i = 15000;
		while(i>0){	i--;}
		i = 20000;
		while(i>0){	i--;}
	}
}

/**
Function will initialise the array containing the pulsewidth of the servos to
the value given in the parameter.
@param uint16 default value to initialise the servo with
**/
void initialiseServoArray(uint16_t pulse_width){
	uint8_t i;
	for(i=0; i<SERVO_NUMBER; i++){
		setServoPWM(i, pulse_width);
	}
}
/**
This function will return the current pulsewidth of the servo passed as an
parameter. Used during interrupts and normal operation!
@param uint8 servo number
@return uint16 pulsewidth of the servo given in the parameter
**/
uint16_t getServoPWM(uint8_t servo){
	return servo_pulse[servo];
}

/**
Function will change the pulsewdith of the servo to the new value passed on in
the parameter.
This function will check that all parameters are with in range.
@param uint8 servo number
@param uint16 pulsewidth of the servo , the range is between 2260 (0.8ms) to
MAX_PULSE(2.2ms)
**/
void setServoPWM(uint8_t servo, uint16_t pulse_width){
	if(servo < SERVO_NUMBER){
		if(pulse_width >= MAX_PULSE){
			pulse_width = MAX_PULSE;
		}else if(pulse_width <= MIN_PULSE){
			pulse_width = MIN_PULSE;
		}
		servo_pulse[servo] = pulse_width;
	}
}

/**
Function will initialise the MSP430 for the PWM board. This involves
initialising the main oscillator, settings ports for IO, putting the USI
in I2C mode, etc
**/
void initialise_PwmBoard(void){
	
	/**Initialising MSP430**/
	WDTCTL = WDTCTL_INIT;	//Initializing watchdog timer
	DCOCTL = CALDCO_INIT;	//Initializing clock source
	BCSCTL1 = CALBC1_INIT;

	P1OUT = P1OUT_INIT;		//Init output data of port1&2
    P2OUT  = P2OUT_INIT;

	P1REN |= P1REN_INIT;	//Init something ???

    P1SEL  = P1SEL_INIT;	//Select port or module -function on port1
    P2SEL  = P2SEL_INIT;	//Select port or module -function on port2d

    P1DIR  = P1DIR_INIT;	//Init port direction register of port1
    P2DIR  = P2DIR_INIT;	//Init port direction register of port2

	P1IES  = P1IES_INIT;	//init port interrupts
	P2IES  = P2IES_INIT;
    P1IE   = P1IE_INIT;
    P2IE   = P2IE_INIT;
	
	/**Initialising program**/
	//set all servos to there ~1.5ms position
	initialiseServoArray(MIDDLE_PULSE);
	
	//TACCR0 interrupts after 20ms
	TACCR0 = PERIOD;
	//TACCR1 interrupts at end of pulse for servo 0
	TACCR1 = getServoPWM(0);

	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module
	
	//setting timer A to count up to TACCR0, and also turn on interrupts
	TACTL = TASSEL_SMCLK|MC_UPTO_CCR0|ID_DIV2|TAIE;

	current_servo = 0;
	initialise_i2c();
	enable_i2c();
	eint(); //enable interrupts
}

/**
This function sweeps all servos between two hardcoded arbitrary positions
**/
uint8_t channel = 0; //keeps track of servo number
void sweepServo(void){
		channel =0;
		delay_MS(50);
		while(channel<SERVO_NUMBER){
			setServoPWM(channel, (uint16_t)(2.3*TICKS_PER_MS));
			channel++;
		}
		channel = 0;
		delay_MS(50);
		while(channel<SERVO_NUMBER){
			setServoPWM(channel, (uint16_t)(0.7*TICKS_PER_MS));
			channel++;
		}
}

 /**
 Main function
 **/
int main(void) {
	//int static unused_initialized_variable_to_make_gdb_happy = 1;
	initialise_PwmBoard();
    while (1){
		if((USICTL1 & USIIFG) || (USICTL1 & USISTTIFG)){
		polled_i2c();
		}
//		sweepServo();
	}
}

/**
ISR for TACCR0. Is called at the end of the pulse period ~20ms, 
it resets TIMERA, resets current_servo, and put servo0 pin high
**/
interrupt (TIMERA0_VECTOR) isr_TACR0(void){
	current_servo = 0;
	P1OUT = (0xFE);
}
/**
ISR for TACCR1, TACCR2 (not available on F2012) and overflow. 
It is called at the end of every pulse ~0.8-2.2ms.
Will set the current servo pin low and set the next servo pin high. 
**/
interrupt (TIMERA1_VECTOR) isr_TAIV(void){ //both for period interrupt and
	switch( TAIV ){
		case  2: // CCR1
			current_servo++;
			//checks that there is a next servo!
			if(current_servo<SERVO_NUMBER){
				//get servo# pulse width, increment cc1 by its width
				TACCR1 += getServoPWM(current_servo);
				P1OUT = ~(0x1<<current_servo); //set servo# output high
			}else{
				P1OUT = (0xFF);
				TACCR1 = getServoPWM(0); //reset CC1 to pulse Servo0
			}
			break;
		case  4: break;       // CCR2 is not present on the F2013
		case 10: break;       // overflow, will occur when

 }
}

/**
ISR for USI. Will process interrupts for the i2c and update servos with new data
DISABLED because of I2C spam
**/
/**
interrupt (USI_VECTOR) isr_USI(void){ //interrupt for i2c
	//eint();
	uint8_t number_of_data;
	uint8_t * data;
	isr_usi ();
	if(available_i2c_data()){
		number_of_data = available_i2c_data();
		data = get_i2cData();
		setServoPWM(data[0], (MIN_PULSE + 25*(uint16_t)data[1])); //takes values from 0-135
	}
}**/

void polled_i2c(void){
	uint8_t number_of_data;
	uint8_t * data;
	isr_usi ();
	if(available_i2c_data()){
		number_of_data = available_i2c_data();
		data = get_i2cData();
		setServoPWM(data[0], (MIN_PULSE + 25*(uint16_t)data[1])); //takes values from 0-135
	}
}

interrupt (NOVECTOR) IntServiceRoutine(void)
{

}



