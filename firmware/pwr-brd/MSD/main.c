/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        main.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the �Company�) for its PICmicro� Microcontroller is intended and
 * supplied to you, the Company�s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       11/19/04    Original.
 ********************************************************************/

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
//#include <p18f4550.h>
//#include "system\sdcard\sdcard.h"
#include "system\typedefs.h"                        // Required
#include "system\usb\usb.h"                         // Required
#include "io_cfg.h"                                 // Required

#include "system\usb\usb_compile_time_validation.h" // Optional
//#include "user\user_mouse.h"                        // Modifiable

#include <i2c.h>
#include <usart.h>
#include "musart.h"
#include <stdlib.h>
#include <adc.h>

/** V A R I A B L E S ********************************************************/
#pragma udata

long int startupdel;
int bcount;
char outstr[10];
char dump2;


/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void USBTasks(void);
void delay(int time);
void i2cservice(void);
/** V E C T O R  R E M A P P I N G *******************************************/

extern void _startup (void);        // See c018i.c in your C18 compiler dir
#pragma code _RESET_INTERRUPT_VECTOR = 0x000020
void _reset (void)
{
    _asm goto _startup _endasm
}
#pragma code

/** D E C L A R A T I O N S **************************************************/
#pragma code
/******************************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *****************************************************************************/
void main(void)
{
    InitializeSystem();


	TRISBbits.TRISB0=1; //Set to inputs for the SPI module
	TRISBbits.TRISB1=1;

	SSPSTATbits.CKE=1; //Enable SMBus specific inputs
	SSPSTATbits.SMP=1; //Slew rate control for 100khz operation
    	
    	SSPCON2bits.SEN = 0; //Disable clock stretching
  	SSPCON2bits.RSEN = 0; //Masking disabled
	SSPCON2bits.PEN=0; //Next 4 bits are an address mask. 0000 means no mask
	SSPCON2bits.RCEN=0;
	SSPCON2bits.ACKEN=0;
	SSPCON2bits.ACKDT=0;
  	//SSPCON2bits.ACKSTAT; //Unused in slave
  	SSPCON2bits.GCEN=0; //Disable interrupt on address 0000h
 
  	SSPCON1bits.SSPM0=0; //0110 = 7 bit I2C slave mode without interrupts. 1110(3-0) for interrupts
  	SSPCON1bits.SSPM1=1;
  	SSPCON1bits.SSPM2=1;
  	SSPCON1bits.SSPM3=0;
  	SSPCON1bits.CKP=1; //Release clock if clock stretching
  	SSPCON1bits.SSPEN=1; //Enable I2C!
  	//SSPCON1bits.SSPOV; //Receive overflow indicator
  	//SSPCON1bits.WCOL; //Write collision detect bit

	SSPADD=0x55<<1; //55 used in slug software
	TRISD=0x0F;
	PORTD = 0x55;

while(1)
{
	manage_usart();
	if (PIR1bits.SSPIF) 
	{
		PORTD = PORTD + 16;
		dump2 = SSPBUF;
		mputcharUSART(dump2);
		PIR1bits.SSPIF=0;
		
	}
		
	//PIR1bits.SSPIF=0;
	//SSPCON1bits.SSPOV=0;
	
	//while(BusyADC());
	//itoa(Read,&outstr);
	//putsUSART(outstr);
	
}




	



//while(1)
	
		/*
		
			while(BusyUSART());
			WriteUSART(10);
			while(BusyUSART());
			WriteUSART(13);
			
			SetChanADC(ADC_CH0);
			ConvertADC();
			while(BusyADC());
			itoa(ReadADC(),&outstr);
			putsUSART(outstr);
			
			WriteUSART(" ");
			
			SetChanADC(ADC_CH1);
			ConvertADC();
			while(BusyADC());
			itoa(ReadADC(),&outstr);
			putsUSART(outstr);
		
		*/

		//PORTC=~PORTC;
		//PORTD=~PORTD;
//		PORTD=bcount << 2;
//		itoa(bcount,&outstr);
//		putsUSART(outstr);
//		WriteUSART(PORTD);
	
//		delay(5);
//	}	
    //OpenI2C(,);  
    while(1)
	    {
		
	     	USBTasks();         // USB Tasks
	        i2cservice();
	        ProcessIO();        // See msd.c & msd.h
	    } //end while
}//end main

/******************************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization routine.
 *                  All required USB initialization routines are called from
 *                  here.
 *
 *                  User application initialization routine should also be
 *                  called from here.                  
 *
 * Note:            None
 *****************************************************************************/
static void InitializeSystem(void)
{
	//ecssr init routine sets io and turns on slug
	//rc0 slug boot switch blip low to boot

//rd0-4 switch
//rd5-7 leds
//re0 - big power motor fet thing
//re1 - fet control servo rail
//re2 - fet slug rail

	PORTC|=0x01; // MUST BE set BEFORE UNTRISTATING ELSE SLUG BOOT!!!
	TRISC=0XFE;// make slug pin Out
	TRISE = 0;
	PORTE = 0b111; // turn all power rails on
	TRISD|=0x0F;

	delay(20);
	PORTCbits.RC0=0;// blip slug
	delay(20);
	PORTCbits.RC0=1; // never press the button, ever!! (dont hold down)

// end tempfw inserts

    TRISC|=0x20;
    OpenUSART(
	USART_TX_INT_OFF &
	USART_RX_INT_OFF &
	USART_ASYNCH_MODE &
	USART_EIGHT_BIT &
	USART_CONT_RX & 
	USART_BRGH_HIGH, // checked, this does mean bit brgh bit set
	152);  // this is 19200
	
// for 115200 brg = 25.041
//for 9600 brg = 312.6
//52.08 for 57600 baud
// this calculation was found to be rubbish, by for loop found rangfe to be 147-157dec, so using 152 (for 19200)
	//TRISC=0X00;
	
	
	    OpenADC( ADC_FOSC_64 &
	ADC_RIGHT_JUST &
	ADC_20_TAD,
	
	ADC_CH0 &
	ADC_INT_OFF &
	ADC_VREFPLUS_VDD &
	ADC_VREFMINUS_VSS
	, 0x0D );// this is to A/D pins ref from rails
	


   // ADCON1 |= 0x0F;                 // Default all pins to digital
    
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See io_cfg.h
    #endif
    
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;
    #endif
    mInitializeUSBDriver();         // See usbdrv.h
    SDCardInit();                     // See msd.c & .h

}//end InitializeSystem

/******************************************************************************
 * Function:        void USBTasks(void)
 *
 * PreCondition:    InitializeSystem has been called.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Service loop for USB tasks.
 *
 * Note:            None
 *****************************************************************************/
void USBTasks(void)
{
    /*
     * Servicing Hardware
     */
    USBCheckBusStatus();                    // Must use polling method
    if(UCFGbits.UTEYE!=1)
        USBDriverService();                 // Interrupt or polling method

}// end USBTasks

void i2cservice(void)
{
	return(0);
}



void delay(int time)
{
	int sponge =0;
	//good time about 20
	startupdel=0;
	
	for (startupdel=0;startupdel<(time*100);startupdel++)
	{
		for(sponge=0;sponge<250;sponge++);
	}

}

           
/** EOF main.c ***************************************************************/

