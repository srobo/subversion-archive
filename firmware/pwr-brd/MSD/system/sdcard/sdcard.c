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

extern volatile far byte msd_buffer[512]; 
/* Where the SD Card CSD register was read into */
/* TODO: Remove this! */
CSD gblCSDReg;			

#pragma udata

typedef enum
{
    GO_IDLE_STATE,
    SEND_OP_COND,         
    SEND_CSD,
    SEND_CID,
    STOP_TRANSMISSION,
    SEND_STATUS,
    SET_BLOCKLEN,
    READ_SINGLE_BLOCK,
    READ_MULTI_BLOCK,
    WRITE_SINGLE_BLOCK,
    WRITE_MULTI_BLOCK,
	TAG_SECTOR_START,
	TAG_SECTOR_END,
	UNTAG_SECTOR,
    TAG_ERASE_GRP_START,
    TAG_ERASE_GRP_END,
    UNTAG_ERASE_GRP,
    ERASE,
    LOCK_UNLOCK,
    SD_APP_OP_COND,
    APP_CMD,
    READ_OCR,
    CRC_ON_OFF
}sdmmc_cmd;


/*********************************************************************
 * sdmmc_cmdtable
 * - Provides information for all the sdmmc commands that we support
 * 
 * Notes: We turn off the CRC as soon as possible, so the commands with
 *        0xFF don't need to be calculated in runtime 
 *********************************************************************/
const rom typSDC_CMD sdmmc_cmdtable[] =
{
    // cmd                  crc   response
    {cmdGO_IDLE_STATE,      0x95, R1, NODATA},
    {cmdSEND_OP_COND,       0xF9, R1, NODATA},
	{cmdSEND_CSD,           0xAF, R1, MOREDATA},
    {cmdSEND_CID,           0x1B, R1, MOREDATA},
    {cmdSTOP_TRANSMISSION,  0xC3, R1, NODATA},
    {cmdSEND_STATUS,        0xAF, R2, NODATA},
    {cmdSET_BLOCKLEN,       0xFF, R1, NODATA},
    {cmdREAD_SINGLE_BLOCK,  0xFF, R1, MOREDATA},
    {cmdREAD_MULTI_BLOCK,   0xFF, R1, MOREDATA},
    {cmdWRITE_SINGLE_BLOCK, 0xFF, R1, MOREDATA},
    {cmdWRITE_MULTI_BLOCK,  0xFF, R1, MOREDATA}, 
    {cmdTAG_SECTOR_START,   0xFF, R1, NODATA},
    {cmdTAG_SECTOR_END,     0xFF, R1, NODATA},
    {cmdUNTAG_SECTOR,       0xFF, R1, NODATA},
    {cmdTAG_ERASE_GRP_START, 0xFF, R1, NODATA},
    {cmdTAG_ERASE_GRP_END,  0xFF, R1, NODATA},
    {cmdUNTAG_ERASE_GRP,    0xFF, R1, NODATA},
    {cmdERASE,              0xDF, R1b, NODATA},
    {cmdLOCK_UNLOCK,        0x89, R1b, NODATA},  
    {cmdSD_APP_OP_COND,     0xE5, R1, NODATA},
    {cmdAPP_CMD,            0x73, R1, NODATA},
    {cmdREAD_OCR,           0x25, R3, NODATA},
    {cmdCRC_ON_OFF,         0x25, R1, NODATA}
};


/** P R I V A T E  P R O T O T Y P E S ***************************************/
void Delayms(byte);
byte MediaDetect(void);
SDC_RESPONSE SendSDCCmd(byte, dword);
byte ReadMedia(void);

/** D E C L A R A T I O N S **************************************************/
#pragma code

/******************************************************************************
 * Function:        BYTE MediaDetect(void)
 *
 * PreCondition:    SocketInitialize function has been executed.
 *
 * Input:           void
 *                  
 * Output:          TRUE   - Card detected
 *                  FALSE   - No card detected
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 *****************************************************************************/
byte MediaDetect()
{
    // give it a debounce here
    Delayms(10);
    
	return(1);//force return that a card really exists even tho its a lie! tb	    
    //return(!MEDIA_CD);  //mediacd is rb4 connected to the switch presumably
}//end MediaDetect


int DetectSDCard(void)
{ 
//	SDC_ON=1;                     // Turned the power for the card ON (RB4 signal available)
	if (MEDIA_CD)				// RB4==1 ---> SD Card not present 				 
		return 0;
	else 
		return 1; 	     
}	                                 
     
/******************************************************************************
 * Function:        SDC_RESPONSE SendSDCCmd(BYTE cmd, DWORD address)
 *
 *
 * Input:           None
 *                  
 * Output:          response            - Response from the card
 *                                      - 0x00 or 0x01 Command received 
 *                                        successfully, else, command failed
 *                  -Bit 0              - In idle state if 1
 *                  -Bit 1              - Erase Reset if 1
 *                  -Bit 2              - Illgal Command if 1
 *                  -Bit 3              - Com CRC Error if 1
 *                  -Bit 4              - Erase Sequence Error if 1
 *                  -Bit 5              - Address Error if 1
 *                  -Bit 6              - Parameter Error if 1
 *                  -Bit 7              - Not used, always '0'
 *
 * Side Effects:    None
 *
 * Overview:        SendSDCCmd prepares the command packet and sends it out
 *                  over the SPI interface. Response data of type 'R1' or 'R2' for SEND_STATUS command (see
 *                  SD or MMC product manuals) is returned.
 *
 * Note:            SDC_CS is not set at the end of this function.
 *                  if the command has no data stage, call macro
 *                  mSendMediaCmd_NoData(), it reasserts SDC_CS to 1.
 *                  If the command has a data stage, SDC_CS must be
 *                  reasserted after the data transfer stage is complete.
 *                  See SectorRead and SectorWrite for examples.
 *****************************************************************************/
SDC_RESPONSE SendSDCCmd(byte cmd, dword address)
{        
    word timeout = 8;
	byte index;
	SDC_RESPONSE response;
	CMD_PACKET CmdPacket;
	    
    SDC_CS = 0;                           //Card Select
    
    // Copy over data
    CmdPacket.cmd        = sdmmc_cmdtable[cmd].CmdCode;
    CmdPacket.address    = address;
    CmdPacket.crc        = sdmmc_cmdtable[cmd].CRC;       // Calc CRC here
    
    
    WriteSPI(CmdPacket.cmd);                //Send Command
   	WriteSPI(CmdPacket.addr3);              //Most Significant Byte
    WriteSPI(CmdPacket.addr2);
   	WriteSPI(CmdPacket.addr1);
    WriteSPI(CmdPacket.addr0);              //Least Significant Byte
    WriteSPI(CmdPacket.crc);                //Send CRC

    // see if  we are going to get a response    
    if(sdmmc_cmdtable[cmd].responsetype == R1 || sdmmc_cmdtable[cmd].responsetype == R1b)
    {
        do
        {
            response.r1._byte = ReadMedia();
            timeout--;
        }while((response.r1._byte == 0xFF) && (timeout != 0));
    }
    else if(sdmmc_cmdtable[cmd].responsetype == R2)
    {
        ReadMedia();
        
        response.r2._byte1 = ReadMedia();
        response.r2._byte0 = ReadMedia();
    }
    
    if(sdmmc_cmdtable[cmd].responsetype == R1b)
    {
		response.r1._byte = 0x00;
		
		for(index =0; index < 0xFF && response.r1._byte == 0x00; index++)
		{
	        timeout = 0xFFFF;
	           
	        do
	        {
	            response.r1._byte = ReadMedia();
	            timeout--;
	        }while((response.r1._byte == 0x00) && (timeout != 0)); 
		}
    }
        
    mSend8ClkCycles();                      //Required clocking (see spec)
 
    // see if we are expecting data or not
    if(!(sdmmc_cmdtable[cmd].moredataexpected))
        SDC_CS = 1;
    
    return(response);        
}

/******************************************************************************
 * Function:        BYTE ReadMedia(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          BYTE    - One byte of data read in from SPI port
 *
 * Side Effects:    None
 *
 * Overview:        ReadMedia reads in one byte of data while sending out 0xFF
 *
 * Note:            Could not use ReadSPI because it initializes SSPBUF to
 *                  0x00. The card expects 0xFF (see spec).
 *****************************************************************************/
byte ReadMedia(void)
{
    SSPBUF = 0xFF;                              //Data Out - Logic ones
    while(!SSPSTATbits.BF);                     //Wait until cycle complete
    return(SSPBUF);                             //Return with byte read 
}//end ReadMedia

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
SDC_Error SectorRead(dword sector_addr, byte* buffer)
{
    word index;
    SDC_RESPONSE    response;
    byte data_token;
    SDC_Error status = sdcValid;

	#ifdef STATUSLED
	STRTRIS = OUTPUT;
	STRLED = 1;
	#endif

    // send the cmd
    response = SendSDCCmd(READ_SINGLE_BLOCK,(sector_addr << 9));

    // Make sure the command was accepted
    if(response.r1._byte != 0x00)
    {
        status = sdcCardBadCmd;
    }
    else
    {
        index = 0x2FF;                                     
     
        //Now, must wait for the start token of data block   
        do
        {
            data_token = ReadMedia();
            index--;
        }while((data_token == SDC_FLOATING_BUS) && (index != 0));
    
        // Hopefully that zero is the datatoken 
        if((index == 0) || (data_token != DATA_START_TOKEN))
            status = sdcCardTimeout;
        else
        {
            for(index = 0; index < SDC_SECTOR_SIZE; index++)
                                                 //Reads in 512-byte of data
			{
					buffer[index] = ReadMedia();
            }  
            // Now ensure CRC    
            mReadCRC();                          //Read 2 bytes of CRC
            //status = mmcCardCRCError;
        }
        
        mSend8ClkCycles();                       //Required clocking (see spec)
    }

    SDC_CS = 1;

	#ifdef STATUSLED
	STRLED = 0;
	#endif

    return(status);
}//end SectorRead

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
    word index;
    byte data_response;
	SDC_RESPONSE    response; 
    SDC_Error status = sdcValid;
  	
	#ifdef STATUSLED
	STWTRIS = OUTPUT;
	STWLED = 1;
	#endif
	
    // send the cmd
    response = SendSDCCmd(WRITE_SINGLE_BLOCK,(sector_addr << 9));
    
    // see if it was accepted
    if(response.r1._byte != 0x00)
        status = sdcCardBadCmd;    
    else
    {
        WriteSPI(DATA_START_TOKEN);               //Send data start token
        
        for(index = 0; index < 512; index++)      //Send 512 bytes of data
            WriteSPI(buffer[index]);
            
        // calc crc    
        mSendCRC();                               //Send 2 bytes of CRC
        
        data_response = ReadMedia();              //Read response
        
        if((data_response & 0x0F) != DATA_ACCEPTED)
        {
            status = sdcCardDataRejected;
        }
        else
        {
            index = 0;                            //using i as a timeout counter
            
            do                                    //Wait for write completion
            {
                data_response = ReadMedia();
                index++;
            }while((data_response == 0x00) && (index != 0));
            
            if(index == 0)                        //if timeout first
                status = sdcCardTimeout;
        }
        
        mSend8ClkCycles();        
    }
    
    SDC_CS = 1;
    
	#ifdef STATUSLED
	STWLED = 0;
	#endif
	
	
  	return(status);
} //end SectorWrite


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
    if(MEDIA_WD) return TRUE;
    else return FALSE;
}

/******************************************************************************
 * Function:        ShutdownMedia(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          Node
 *
 * Side Effects:    None
 *
 * Overview:        Closes the SPI bus and deselect the device. 
 *					(Turn off the power if possible.)
 *****************************************************************************/
void ShutdownMedia(void)
{
    // close the spi bus
    CloseSPI();
    
    // deselect the device
    SDC_CS = 1;                               
    
    // Turn off the card
    // SDC_OFF;        
}


