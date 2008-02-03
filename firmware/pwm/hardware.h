#ifndef HARDWARE_H
#define HARDWARE_H

#include <io.h>
#include <signal.h>
#include <iomacros.h>

/**
Main oscillator settings. Setup registers DCOCTL and BCSCTL1 so DCOCLK
is main clock and is at 12Mhz
**/
#define CALDCO_INIT		CALDCO_12MHZ_
#define CALBC1_INIT		CALBC1_12MHZ_
/**
Port Output Register 'P1OUT, P2OUT':
**/
#define P1OUT_INIT		0xC0	// P1.6 & P1.7 Pullups for i2c REALLY?
#define P2OUT_INIT		0		// Init Output data of port2

/**
Does something to initialise P1REN
// 1.6 and 1.7 pullups
**/
#define P1REN_INIT		0x00

/**
Port Direction Register 'P1DIR, P2DIR'; 
this sets the port directions. Port1.6 and 1.7 are set to input because they will be used with the i2c. Its unclear from the manual if it must be done
**/
#define P1DIR_INIT      0x3f                    // Init of Port1 Data-Direction Reg (Out=1 / Inp=0) current setting 00111111
#define P2DIR_INIT      0x7f                    // Init of Port2 Data-Direction Reg (Out=1 / Inp=0) 

//Selection of Port or Module -Function on the Pins 'P1SEL, P2SEL'
#define P1SEL_INIT      0xC0                     // P1-Modules p1.6-1.7 is i2c
#define P2SEL_INIT      0                       // P2-Modules: (1 = peripheral 0=IO)

//Interrupt capabilities of P1 and P2
#define RAIL_MONITOR_PIN 0x80

#define P1IE_INIT       0                       // Interrupt Enable (0=dis 1=enabled)
#define P2IE_INIT       RAIL_MONITOR_PIN                       // Interrupt Enable (0=dis 1=enabled)
#define P1IES_INIT      0                       // Interrupt Edge Select (0=pos 1=neg)
#define P2IES_INIT      RAIL_MONITOR_PIN                       // Interrupt Edge Select (0=pos 1=neg)

//Disable watchdog i think but comment says other wise ???system clock in div by 2
#define WDTCTL_INIT     WDTPW|WDTHOLD

#endif //HARDWARE_H
