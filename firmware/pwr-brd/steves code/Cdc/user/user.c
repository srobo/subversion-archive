/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        user.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       11/19/04    Original.
 * Stephen English		29/03/07    Modification for use in CPU Teacher
 * Tom Bennellick (ECSSR) 01/10/07  mod for ecssr integration
 ********************************************************************/
 
/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include "system\typedefs.h"

#include "system\usb\usb.h"

#include "io_cfg.h"             // I/O pin mapping
#include "user\user.h"

/** V A R I A B L E S ********************************************************/

#define DecodeHexChar(n) (n < 65 ? n-48 : n-55)

//unsigned char just_data[32];

/** P R I V A T E  P R O T O T Y P E S ***************************************/
//void InitializeUSART(void);
//unsigned char DecodeHex(unsigned char *buffer);
//void MakeHex(unsigned char *buffer, unsigned char data);

/** D E C L A R A T I O N S **************************************************/

/*
void UserInit(void)
{
    //PORTA
	ADCON0bits.CHS0 = 0;
	ADCON0bits.CHS1 = 0;
	ADCON0bits.CHS2 = 0;
	ADCON0bits.CHS3 = 0; //Select channel 0
	ADCON0bits.ADON = 1;
	ADCON1bits.VCFG0 = 0; //Use Vss & Vdd for references
	ADCON1bits.VCFG1 = 0;
	ADCON1bits.PCFG3 = 1;
	ADCON1bits.PCFG2 = 1;
	ADCON1bits.PCFG1 = 1;
	ADCON1bits.PCFG0 = 1; //AN0 only
	ADCON2bits.ADFM = 0; //Left justified result
	ADCON2bits.ACQT2 = 1;
	ADCON2bits.ACQT1 = 0;
	ADCON2bits.ACQT0 = 1; //A/D Acquisition time select bits. 101 = 12 Tad. Is this right? Who knows?!?
	ADCON2bits.ADCS2 = 1;
	ADCON2bits.ADCS1 = 1;
	ADCON2bits.ADCS0 = 0; //64 Tosc - Hope this is right!
	
	CMCON = 0; //Disable Comparators
	TRISA = 0xFF; //All Output
	PIR1bits.ADIF = 0;
	
	//PORTB
	INTCONbits.INT0IE = 0;
	INTCONbits.RBIE = 0;
	INTCONbits.INT0IF = 0;
	INTCONbits.RBIF = 0;
	INTCON2bits.RBPU = 1;
	INTCON2bits.INTEDG0 = 0;
	INTCON2bits.INTEDG1 = 0;
	INTCON2bits.INTEDG2 = 0;
	INTCON2bits.RBIP = 0;
	SPPCONbits.SPPEN = 0;
	SPPCFGbits.CSEN = 0;
	TRISB = 0; //All outputs
	
	//PORTC & Serial	
	TRISC = 0; //This doesn't affect the USB pins

	//PORTD
	CCP1CONbits.P1M1 = 0;
	CCP1CONbits.P1M0 = 0;
	SPPCONbits.SPPEN = 0;
	LATE = 0x80; //Enable pullups
	TRISD = 0xFF;

	//PORTE
	ADCON1bits.VCFG1 = 0;
	ADCON1bits.VCFG0 = 0;
	ADCON1bits.PCFG3 = 0;
	ADCON1bits.PCFG2 = 0;
	ADCON1bits.PCFG1 = 0;
	ADCON1bits.PCFG0 = 0;
	TRISE = 0;
	
	//10S Timer
	T0CONbits.TMR0ON = 1; //Enable timer
	T0CONbits.T08BIT = 0; //16bit
	T0CONbits.T0CS = 0; //Clock on instruction cycle
	T0CONbits.T0SE = 0; //Low to high transition. Not used
	T0CONbits.PSA = 0; //User Prescaler
	T0CONbits.T0PS2 = 1;
	T0CONbits.T0PS1 = 1;
	T0CONbits.T0PS0 = 1; //1:256 prescaler. 1 tick = 1.39s
	
}//end UserInit

*/

/*
void InitializeUSART(void)
{
    TRISCbits.TRISC7=1; // RX
    TRISCbits.TRISC6=0; // TX
    SPBRG = 0x71;
    SPBRGH = 0x02;      // 0x0271 for 48MHz -> 19200 baud
    TXSTA = 0x24;       // TX enable BRGH=1
    RCSTA = 0x90;       // continuous RX
    BAUDCON = 0x08;     // BRG16 = 1
}//end InitializeUSART
*/


/******************************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user routines.
 *                  It is a mixture of both USB and non-USB tasks.
 *
 * Note:            None
 *****************************************************************************/

/*
void ProcessIO(void)
{
	static unsigned char a = 0;
	unsigned char b=0;
	static unsigned char bufid = 0;
	static unsigned char bufpos = 0;
	static unsigned char needtosend = 0;

	//if(INTCONbits.TMR0IF){
	if(ADCON0bits.GO_DONE == 0){
		//INTCONbits.TMR0IF = 0;
		a ^= 0x01;
		ADCON0bits.GO_DONE = 1;
		
		if(bufid==0){
			bufa[bufpos++] = ADRESH;
			bufa[bufpos++] = ADRESL;
		
			if(bufpos == 254){
				if(needtosend > 0){
					b = 1;
					bufpos = 252;
				} else {
					bufpos = 0;
					bufid = 1;
					needtosend = 10;
				}
			}
		} else {
			bufb[bufpos++] = ADRESH;
			bufb[bufpos++] = ADRESL;
		
			if(bufpos == 254){
				if(needtosend > 0){
					b = 1;
					bufpos = 252;
				} else {
					bufpos = 0;
					bufid = 0;
					needtosend = 11;
				}
			}			
		}
	}
	
	if(needtosend > 0)
		if(mUSBUSARTIsTxTrfReady()){
			if(needtosend == 10){
				mUSBUSARTTxRam(bufa, 254);
			} else {
				mUSBUSARTTxRam(bufb, 254);
			}
			b = 0;
			needtosend = 0;
		}
	
	PORTB = ~(a | (b << 1));
}
*/