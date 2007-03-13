#ifndef HARDWARE_H
#define HARDWARE_H

#include <io.h>
#include <signal.h>
#include <iomacros.h>



//Port Output Register 'P1OUT, P2OUT':
#define P1OUT_INIT      0                      // Init Output data of port1
#define P2OUT_INIT      0                       // Init Output data of port2

//Port Direction Register 'P1DIR, P2DIR':
#define P1DIR_INIT      0x3f                    // Init of Port1 Data-Direction Reg (Out=1 / Inp=0) setting P1.6-1.7 to in
#define P2DIR_INIT      0xff                    // Init of Port2 Data-Direction Reg (Out=1 / Inp=0)

//Selection of Port or Module -Function on the Pins 'P1SEL, P2SEL'
#define P1SEL_INIT      0xC0                        // P1-Modules p1.6-1.7 is i2c
#define P2SEL_INIT      0                       // P2-Modules:

//Interrupt capabilities of P1 and P2
#define P1IE_INIT       0                       // Interrupt Enable (0=dis 1=enabled)
#define P2IE_INIT       0                       // Interrupt Enable (0=dis 1=enabled)
#define P1IES_INIT      0                       // Interrupt Edge Select (0=pos 1=neg)
#define P2IES_INIT      0                       // Interrupt Edge Select (0=pos 1=neg)

 //system clock in div by 2
#define WDTCTL_INIT     WDTPW|WDTHOLD

#define PERIOD 56500
#define TICKS_PER_MS PERIOD/20
#define SERVO_NUMBER 6

void initialiseServoArray(unsigned int pulse_width);
unsigned int getServoPWM(unsigned char servo);
void setServoPWM(unsigned char servo, unsigned int pulse_width);



#endif //HARDWARE_H
