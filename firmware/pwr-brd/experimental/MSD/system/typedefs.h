/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        typedefs.h
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
 ********************************************************************/

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#define SR_BLOCKLEN 512

typedef unsigned char   byte;           // 8-bit
typedef unsigned int    word;           // 16-bit
typedef unsigned long   dword;          // 32-bit

typedef union _BYTE
{
    byte _byte;
    struct
    {
        unsigned b0:1;
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    };
} BYTE;

typedef union _WORD
{
    word _word;
    struct
    {
        byte byte0;
        byte byte1;
    };
    struct
    {
        BYTE Byte0;
        BYTE Byte1;
    };
    struct
    {
        BYTE LowB;
        BYTE HighB;
    };
    struct
    {
        byte v[2];
    };
} WORD;
#define LSB(a)      ((a).v[0])
#define MSB(a)      ((a).v[1])

typedef union _DWORD
{
    dword _dword;
    struct
    {
        byte byte0;
        byte byte1;
        byte byte2;
        byte byte3;
    };
    struct
    {
        word word0;
        word word1;
    };
    struct
    {
        BYTE Byte0;
        BYTE Byte1;
        BYTE Byte2;
        BYTE Byte3;
    };
    struct
    {
        WORD Word0;
        WORD Word1;
    };
    struct
    {
        byte v[4];
    };
} DWORD;


#define LOWER_LSB(a)    ((a).v[0])
#define LOWER_MSB(a)    ((a).v[1])
#define UPPER_LSB(a)    ((a).v[2])
#define UPPER_MSB(a)    ((a).v[3])

typedef void(*pFunc)(void);

typedef union _POINTER
{
    struct
    {
        byte bLow;
        byte bHigh;
        //byte bUpper;
    };
    word _word;                         // bLow & bHigh
    
    //pFunc _pFunc;                       // Usage: ptr.pFunc(); Init: ptr.pFunc = &<Function>;

    byte* bRam;                         // Ram byte pointer: 2 bytes pointer pointing
                                        // to 1 byte of data
    word* wRam;                         // Ram word poitner: 2 bytes poitner pointing
                                        // to 2 bytes of data

    rom byte* bRom;                     // Size depends on compiler setting
    rom word* wRom;
    //rom near byte* nbRom;               // Near = 2 bytes pointer
    //rom near word* nwRom;
    //rom far byte* fbRom;                // Far = 3 bytes pointer
    //rom far word* fwRom;
} POINTER;

typedef enum _BOOL { FALSE = 0, TRUE } BOOL;

#define OK      TRUE
#define FAIL    FALSE

typedef enum 
{
    sdcValid=0,                 // Everything is golden
    sdcCardInitCommFailure,     // Communication has never been established with card
    sdcCardNotInitFailure,      // Card did not go into an initialization phase
    sdcCardInitTimeout,         // Card initialization has timedout
    sdcCardTypeInvalid,         // Card type was not able to be defined
    sdcCardBadCmd,              // Card did not reconized the command
    sdcCardTimeout,             // Card timedout during a read, write or erase sequence   
    sdcCardCRCError,            // A CRC error has occurred during a read, data should be invalidated
    sdcCardDataRejected,         // Card and data sent's CRC did not match
    sdcEraseTimedOut            // Erase took longer than it should have 
}SDC_Error;

typedef union _SDCstate
{
    struct
	{
        byte isSDMMC : 1;     // set if it is a SDMMC 
        byte isWP    : 1;     // set if it is write protected 
    };
    byte  _byte;
} SDCSTATE;

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
        byte _byte[16];
    };
    struct
    {
        unsigned CSD_STRUCTURE      :2;
        unsigned SPEC_VERS          :4;
        unsigned RESERVED_3         :2;
        
        unsigned TAAC               :8;
        
        unsigned NSAC               :8;
        
        unsigned TRAN_SPEED         :8;
        
        unsigned CCC_H              :8;
        
        unsigned READ_BL_LEN        :4;
        unsigned CCC_L              :4;
        
        
        unsigned READ_BL_PARTIAL    :1;
        unsigned WRITE_BLK_MISALIGN :1;
        unsigned READ_BLK_MISALIGN  :1;
        unsigned DSR_IMP            :1;
        unsigned RESERVED_2         :2;
        unsigned C_SIZE_U           :2;
        
        unsigned C_SIZE_H           :8;
        
        unsigned C_SIZE_L           :2;
        unsigned VDD_R_CURR_MIN     :3;
        unsigned VDD_R_CURR_MAX     :3;
        
        unsigned VDD_W_CUR_MIN      :3;
        unsigned VDD_W_CURR_MAX     :3;
        unsigned C_SIZE_MULT_H      :2;
                
        unsigned C_SIZE_MULT_L      :1;
        unsigned SECTOR_SIZE        :5;
        unsigned ERASE_GRP_SIZE_H   :2;
        
        unsigned ERASE_GRP_SIZE_L   :3;
        unsigned WP_GRP_SIZE        :5;

        unsigned WP_GRP_ENABLE      :1; //bit 016 - 031
        unsigned DEFAULT_ECC        :2;
        unsigned R2W_FACTOR         :3;
        unsigned WRITE_BL_LEN_H     :2;
        
        unsigned WRITE_BL_LEN_L     :2;
        unsigned WRITE_BL_PARTIAL   :1;
        unsigned RESERVED_1         :5;
        
        unsigned FILE_FORMAT_GRP    :1; //bit 008 - 015
        unsigned COPY               :1;
        unsigned PERM_WRITE_PROTECT :1;
        unsigned TMP_WRITE_PROTECT  :1;
        unsigned FILE_FORMAT        :2;
        unsigned ECC                :2;
        
        unsigned CRC                :7; //bit 000 - 007
        unsigned NOT_USED           :1;
    };
} CSD;


#endif //TYPEDEFS_H
