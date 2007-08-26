#include "common.h"
#include "msp430/usci.h"
#include <signal.h>
#include "i2c.h"

void i2c_init( void )
{
    UCB0CTL0 = 0;
    UCB0CTL1 = 0;

    P3SEL = 6; //Enable peripherals on pins 1 and 2
    P3DIR = 6; //This is a complete stab in the dark
    
    //1. Set UCSWRST to hold the USCI in reset:
    //      i2c comms stops
    //      SDA and SCL are high impedence
    //      UCBxI2CSTAT, bits 6-0 are cleared
    //      UCBxTXIE and UCBxRXIE are cleared
    //      UCBxTXIFG and UCBxRXIFG are cleared
    //      All other bits and registers unchanged
    //  Other bits in this register are for master mode

    UCB0CTL1 |= UCSWRST;

    //2. Set UCMODEx to 11 for i2c mode

    UCB0CTL0 |= UCMODE0;
    UCB0CTL0 |= UCMODE1;
    
    //3. Set UCSYNC = 1
    UCB0CTL0 |= UCSYNC;

    //UCMST cleared already

    //5. Set the address in the UCBxI2COA register. UCA10 = 0 for 7 bit
    //addressing

    UCB0I2COA = 0x2A;

    //Enable interrupts
    //Interrupt on start of data
    UCB0I2CIE |= UCSTTIE;
    IFG2 |= UCB0RXIE;

    //Drop down the RESET.
    UCB0CTL1 &= ~UCSWRST;

    return;

}
