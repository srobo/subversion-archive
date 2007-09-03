/******************************************************************************
 *
 *       PIC18 C18 Secure Digital and Multimedia Cards Interface
 *
 ******************************************************************************
 * FileName:        sdcard.h
 * Dependencies:    spi.h typedefs.h, Compiler.h, usbcfg.h
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
#include <spi.h>                    //SPI library functions
#include "system\typedefs.h"
#include "system\Compiler.h"
#include "autofiles\usbcfg.h"

#define FALSE	0
#define TRUE	!FALSE

#define INPUT	1
#define OUTPUT	0

#define MOREDATA    !0
#define NODATA      0
#define SDC_SECTOR_SIZE    512
#define CSD_SIZE 16			// 16 Bytes CSD register

// used in delayms function
#define low(num) (num & 0xFF)
#define high(num) ((num >> 8) & 0xFF)
#define upper(num) ((num >> 16) & 0xFF)

// used in delayms function
#define SYSTEM_CLOCK	(dword)20000000
#define CLKSPERINSTRUCTION (byte) 4
#define TMR1PRESCALER	(byte)    8
#define TMR1OVERHEAD	(byte)    5	
#define MILLISECDELAY   (word)((SYSTEM_CLOCK/CLKSPERINSTRUCTION/TMR1PRESCALER/(word)1000) - TMR1OVERHEAD)

#define SDC_CS              LATBbits.LATB3     
#define SDC_CS_DIR          TRISB3
#define MEDIA_CD            RB4
#define MEDIA_CD_DIR        TRISB4
#define MEDIA_WD			RA4
#define MEDIA_WD_DIR		TRISA4
//#define SDC_ON 				RA5
//#define SDC_ON_DIR			TRISA5

#define STATUSLED
#define STRTRIS TRISD2
#define STWTRIS TRISD3
#define STRLED RD2
#define STWLED RD3
	
/* Command Operands */
#define BLOCKLEN_64                 0x0040
#define BLOCKLEN_128                0x0080
#define BLOCKLEN_256                0x0100
#define BLOCKLEN_512                0x0200

#define SDC_FLOATING_BUS    0xFF
#define SDC_BAD_RESPONSE    SDC_FLOATING_BUS
#define SDC_ILLEGAL_CMD     0x04
#define SDC_GOOD_CMD        0x00

/* Data Token */
#define DATA_START_TOKEN            0xFE
#define DATA_MULT_WRT_START_TOK     0xFC
#define DATA_MULT_WRT_STOP_TOK      0xFD

/* Data Response */
#define DATA_ACCEPTED               0b00000101
#define DATA_CRC_ERR                0b00001011
#define DATA_WRT_ERR                0b00001101

// The SD Card Commands (SPI Bus Mode)
// Bit 7 = 0, Bit 6 = 1, Bits 5-0=Command Code
#define     cmdGO_IDLE_STATE        0x40		//0 is the 6-bit command
#define     cmdSEND_OP_COND         0x41		//1        
#define     cmdSEND_CSD             0x49		//9
#define     cmdSEND_CID             0x4a		//10
#define     cmdSTOP_TRANSMISSION    0x4c		//12
#define     cmdSEND_STATUS          0x4d		//13
#define     cmdSET_BLOCKLEN         0x50		//16
#define     cmdREAD_SINGLE_BLOCK    0x51		//17
#define     cmdREAD_MULTI_BLOCK     0x52		//18
#define     cmdWRITE_SINGLE_BLOCK   0x58		//24    
#define     cmdWRITE_MULTI_BLOCK    0x59		//25
#define     cmdTAG_SECTOR_START     0x60		//32
#define     cmdTAG_SECTOR_END       0x61		//33
#define     cmdUNTAG_SECTOR         0x62		//34
#define     cmdTAG_ERASE_GRP_START  0x63		//35 
#define     cmdTAG_ERASE_GRP_END    0x64		//36
#define     cmdUNTAG_ERASE_GRP      0x65		//37
#define     cmdERASE                0x66		//38
#define     cmdSD_APP_OP_COND       0x69		//41
#define     cmdLOCK_UNLOCK          0x71		//49
#define     cmdAPP_CMD              0x77		//55
#define     cmdREAD_OCR             0x7a		//58
#define     cmdCRC_ON_OFF           0x7b		//59




// the various possible responses
typedef enum
{
    R1,
    R1b,
    R2,
    R3    // we don't use R3 since we don't care about OCR 
}RESP;

typedef union
{
    struct
    {
        byte field[5];
    };
    struct
    {
        byte crc;
        byte addr0;
        byte addr1;
        byte addr2;
        byte addr3;
        byte cmd;
    };
    struct
    {
        unsigned END_BIT:1;
        unsigned CRC7:7;
        dword address;
        byte command;
    };
} CMD_PACKET;

typedef union
{
    byte _byte;
    struct
    {
        unsigned IN_IDLE_STATE:1;
        unsigned ERASE_RESET:1;
        unsigned ILLEGAL_CMD:1;
        unsigned CRC_ERR:1;
        unsigned ERASE_SEQ_ERR:1;
        unsigned ADDRESS_ERR:1;
        unsigned PARAM_ERR:1;
        unsigned B7:1;
    };
} RESPONSE_1;

typedef union
{
    word _word;
    struct
    {
        byte      _byte0;
        byte      _byte1;
    };
    struct
    {
        unsigned IN_IDLE_STATE:1;
        unsigned ERASE_RESET:1;
        unsigned ILLEGAL_CMD:1;
        unsigned CRC_ERR:1;
        unsigned ERASE_SEQ_ERR:1;
        unsigned ADDRESS_ERR:1;
        unsigned PARAM_ERR:1;
        unsigned B7:1;
        unsigned CARD_IS_LOCKED:1;
        unsigned WP_ERASE_SKIP_LK_FAIL:1;
        unsigned ERROR:1;
        unsigned CC_ERROR:1;
        unsigned CARD_ECC_FAIL:1;
        unsigned WP_VIOLATION:1;
        unsigned ERASE_PARAM:1;
        unsigned OUTRANGE_CSD_OVERWRITE:1;
    };
} RESPONSE_2;

typedef union
{
    RESPONSE_1  r1;  
    RESPONSE_2  r2;
}SDC_RESPONSE;

typedef struct
{
    byte    CmdCode;            // the command number
    byte    CRC;            	// the CRC value (CRC's are not required once you turn the option off!)
    RESP    responsetype;   	// the Response Type
    byte    moredataexpected;   // True if more data is expected
} typSDC_CMD;


typedef union
{
    struct
    {
        DWORD _u320;
        DWORD _u321;
        DWORD _u322;
        DWORD _u323;
    };
    struct
    {
        BYTE _byte[16];
    };
    struct
    {
        unsigned 	NOT_USED           	:1;
        unsigned 	CRC                	:7;     
        unsigned 	MDT                	:8;     // Manufacturing Date Code (BCD)
        DWORD 		PSN;    					// Serial Number (PSN)
        unsigned 	PRV                	:8;     // Product Revision
		char		PNM[6];    					// Product Name
        WORD 		OID;    					// OEM/Application ID
        unsigned 	MID                	:8;     // Manufacture ID                        
    };
} CID;

//Macros
#define mSendMediaCmd_NoData()  SendMediaCmd();SDC_CS=1;
#define mReadCRC()              WriteSPI(0xFF);WriteSPI(0xFF);
#define mSendCRC()              WriteSPI(0xFF);WriteSPI(0xFF);
#define mSend8ClkCycles()       WriteSPI(0xFF);

void SocketInitialize(void);
SDC_Error SectorRead(dword, byte*);
SDC_Error SectorWrite(dword, byte*);
SDC_Error CSDRead(void);
int DetectSDCard (void);
byte IsWriteProtected(void);
