/******************************************************************************
 *
 *       PIC18 C18 Secure Digital and Multimedia Cards Interface
 *
 ******************************************************************************
 * FileName:        sdcard.c
 * Dependencies:    sdcard.h
 * Processor:       PIC18
 * Compiler:        C18, 2.4
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
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       02/06/04    Original        Version 1.0
 *****************************************************************************
 * History               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Gurinder Singh 		05/31/05	Edited and Added Functions
 *****************************************************************************/
 
 #include "sdcard.h"
 #include "p18f4550.h"



extern volatile far byte msd_buffer[512]; 
extern unsigned char usbflag;
extern unsigned char data[32];
extern unsigned long sectadd;
extern char tempstat;

unsigned char* sd_outbuf;

#pragma udata

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void Delayms(byte);

/** D E C L A R A T I O N S **************************************************/
#pragma code

void Delayms(byte milliseconds)
{
	T1CON = 0xB0;
	
	//enable interrupt
	TMR1IE = 1;

	do {
		// load timer
		TMR1H = high(0xFFFF - MILLISECDELAY);
		TMR1L = low(0xFFFF - MILLISECDELAY);
	
		// clear flag
		TMR1IF = 0;

		// turn on timer
		TMR1ON = 1;

		// wait for it to be done
		while(!TMR1IF)
		{
			;
		}

		// turn off timer
		TMR1ON = 0;

		// Making a little more accurate
		Nop();
		Nop();

		milliseconds--;
   	}while(milliseconds > 0);

Error:   
	// now turn everything off 
	TMR1ON = 0;
	TMR1IE = 0;
}


/******************************************************************************
 * Function:        SDC_Error SectorRead(DWORD sector_addr, BYTE *buffer)
 *
 * PreCondition:    None
 *
 * Input:           sector_addr - Sector address, each sector contains 512-byte
 *                  buffer      - Buffer where data will be stored, see
 *                                'ram_acs.h' for 'block' definition.
 *                                'Block' is dependent on whether internal or
 *                                external memory is used
 *                  
 * Output:          See SDC_Error.
 *
 * Side Effects:    None
 *
 * Overview:        SectorRead reads 512 bytes of data from the card starting
 *                  at the sector address specified by sector_addr and stores
 *                  them in the location pointed to by 'buffer'.
 *
 * Note:            The card expects the address field in the command packet
 *                  to be byte address. Therefore the sector_addr must first
 *                  be converted to byte address. This is accomplished by
 *                  shifting the address left 9 times.
 *****************************************************************************/
char SectorRead(dword sector_addr, byte* buffer)
{
		int sp;
	unsigned char sectorposition = 0;
	int fill =0;
	char temploop;
    SDC_Error status = sdcValid;
    RD4=1;
    sectadd=sector_addr;
	
	for (fill=0;fill<512;fill++) buffer[fill]=0xfe;



	for (sectorposition=0;sectorposition<2;sectorposition++)
	{
	
		usbflag=(sectorposition|0x40); // set readflag
		
		mputcharUSART('T');
		while(usbflag!=0)
		{				
		i2cservice();
		//manage_usart();	
		}
		mputcharUSART('M');
		for (temploop=0;temploop<32;temploop++) 
				buffer[((unsigned int)sectorposition*32)+(unsigned int)temploop] = data[temploop];// copy i2c to msd buffer
	}
	
	
	
	RD4=0;
	//buffer[0]=1;
	//buffer[20]=0xaa;
    return(0);//(status);
}

/******************************************************************************
 * Function:        SDC_Error SectorWrite(DWORD sector_addr, BYTE *buffer)
 *
 * PreCondition:    None
 *
 * Input:           sector_addr - Sector address, each sector contains 512-byte
 *                  buffer      - Buffer where data will be read, see
 *                                'ram_acs.h' for 'block' definition.
 *                                'Block' is dependent on whether internal or
 *                                external memory is used
 *                  
 * Output:          See SDC_Error.
 *
 * Side Effects:    None
 *
 * Overview:        SectorWrite sends 512 bytes of data from the location
 *                  pointed to by 'buffer' to the card starting
 *                  at the sector address specified by sector_addr.
 *
 * Note:            The card expects the address field in the command packet
 *                  to be byte address. Therefore the sector_addr must first
 *                  be converted to byte address. This is accomplished by
 *                  shifting the address left 9 times.
 *****************************************************************************/
SDC_Error SectorWrite(dword sector_addr, byte* buffer)
{

		int sp;
	unsigned char sectorposition = 0;
	int fill =0;
	char temploop;
    SDC_Error status = sdcValid;
  	


	sectadd=sector_addr;

	for (sectorposition=0;sectorposition<2;sectorposition++)
	{
		usbflag=(sectorposition|0x80); // set writeflag
	
		/* Tell the i2c routines where to find the data to send */
		sd_outbuf = buffer + ((unsigned int)sectorposition*32);
		
		//if(data[0]==0)RD5=1;
		
		while(usbflag!=0)
			i2cservice();	
	}

	
  	return(status);
} 


/******************************************************************************
 * Function:        byte IsWriteProtected()
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          TRUE if Write Protected, FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Check the write protect status of the card
 *****************************************************************************/
byte IsWriteProtected(void)
{
	return FALSE;
}



