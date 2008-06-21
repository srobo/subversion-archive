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
 * (the Company) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Companys customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN AS IS CONDITION. NO WARRANTIES,
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

#define u8 unsigned char
#define u16 unsigned int

#define GOOD 0
#define BAD 1

typedef struct {
    u8 bytestoreadin;
    u8 bytestoreadout;
    void (*docmd)(u8 *data);
} t_command;

long int startupdel;
int bcount;
//char outstr[10];
char dump2;
unsigned char i2cstatus = BAD;
int voltage = 0x5555;// local variables holding results of adc
int current = 0xAAAA;

#define i2c_debug if (0)

u8 data[32]; // size according to smbus spec 
unsigned long sectadd=0xabcdef12;
unsigned char usbflag=0x00; // non zero means usb i2c bridge needs serviceing , maby use to give idea of direction etc. 
unsigned char usbdataused=0;// set by usb code, cleared by i2c code
//unsigned char usbbuf[32]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

extern unsigned char* sd_outbuf;

/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void USBTasks(void);
void delay(int time);
void i2cservice(void);
u8 i2c_smbus_pec(u8 crc, u8 *p, u8 count);
u8 crc8(u8 tmpdata);
void docmd(u8 command, u8 *data);
void swin(void);
void adcserv(void);

void prdbg(char sentinel, char data);

void identify(u8 *data);
void setled(u8 *data);
void checkusb(u8 *data);
void getv(u8 *data);
void geti(u8 *data);
void getdip(u8 *data);
void setrails(u8 *data);
void getrails(u8 *data);
void sendser(u8 *data);
void getusbbuf(u8 *data);
void setusbbuf(u8 *data);
void getsectorlo(u8 *data);
void getsectorhi(u8 *data);
void datagood(u8 *data);

/** V E C T O R  R E M A P P I N G *******************************************/
//						{bytes in, bytesout, function name}
t_command commands[] = {{0, 1,identify}, //0
                        {1, 0,setled},
                        {0, 1,checkusb},//2
                        {0, 2,getv},
                        {0, 2,geti},//4
                        {0, 1,getdip},
                        {1, 0,setrails},//6
	                    {0, 1,getrails},
		                {0,32,getusbbuf},//8
			            {32,0,setusbbuf},
		                {1,0,sendser},//10
			            {0,2,getsectorlo},
				        {0,2,getsectorhi},//12
					    {32,0,setusbbuf},//old not really needed kept for 
						{1, 0,datagood}};//14

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
	char poo;
    InitializeSystem();

    while(1)
    {
        //manage_usart();	
         
        USBTasks();         // USB Tasks 
        i2cservice();    
        ProcessIO();        // See msd.c & msd.h
        //swin();
        /*if ( PORTDbits.RD3)
        {
	         PORTDbits.RD4^=1;
	        adcserv();
	    } 
	    else PORTDbits.RD4=0; */
	    
    } //end while
}//end main

void adcserv(void)
{
	if (!BusyADC())
	{
		// currently this totallyt rodgers the msd code, dunno why!
		
		PORTD=~PORTD;
	
		if(!ADCON0bits.CHS0)
		{
			
			voltage = ReadADC();
			SetChanADC(ADC_CH1);
			ConvertADC();
			
		}
		else
		{
			current = ReadADC();
			SetChanADC(ADC_CH0);
			ConvertADC();
		}
		
		
		
	}
        
       
    return;
        //itoa(ReadADC(),&outstr);
        //strcount =0;
        //while(outstr[strcount]!= 0 ) mputcharUSART(outstr[(strcount++)]);   
}

void i2cservice(void)
{
    static enum states {
        WAIT,
            GOTADDRESSREAD,
            GOTADDRESSWRITE,
            GOTCOMMAND,
            GOTDATA,
            SENTCHECKSUM
    } state = WAIT;

    static u8 adddump;
    static u8 command;
   
    static u8 datacount;
    static u8 datapos;
    static u8 checksum;
    int i;
    

    u8 tmpdata;

    if (PIR1bits.SSPIF){
	    
	    if(SSPCON1bits.SSPOV) // check for buffer overflow and goto to wait
	    {
		    mputcharUSART('.'); 
		    tmpdata = SSPBUF;
		    SSPCON1bits.SSPOV = 0;
		    state = WAIT;
		    return;
		}
		    
		    
	   
	    
	    //mputcharUSART('X');
        if(!SSPSTATbits.D_A) //It's an address byte
        {
	        i2c_debug mputcharUSART('A');
            i2cstatus = BAD; //This is to check abandoned machines later
            adddump = SSPBUF;
             i2c_debug prdbg('>', adddump);
        	PIR1bits.SSPIF = 0;
            if (!SSPSTATbits.R_W) //About to receive something from the slug
            {
	            mputcharUSART('N');
	            datacount = 0;
	            state = GOTADDRESSREAD;
	            checksum = crc8(adddump);
	            datapos = 0;
	            //mputcharUSART('R');
            }
            else //Being asked to send stuff to the slug
           	{
	           	mputcharUSART('C');
	           	datacount = commands[command].bytestoreadout;
	           	checksum = crc8(checksum^(adddump));
	           	
	           	//Clock stretching is currently in effect
	           	//Can have a bit of time to generate data, so do it now
	           	commands[command].docmd(data); //Data filled up ready to send to slug (Maybe)
	           	
	           	state = GOTADDRESSWRITE;
	        	
	       		//mputcharUSART('W');
	       		SSPBUF = data[0];
	       		i2c_debug prdbg('[', data[0]);
	            SSPCON1bits.CKP = 1; //Disable the clock stretching
	            checksum = crc8(checksum^data[0]);
	            datapos = 1;
	      	}
        } else {
            switch(state){
                case GOTADDRESSREAD: //Just received a command
                    command = SSPBUF;
        			PIR1bits.SSPIF = 0;
                    //command = tmpdata;
                    mputcharUSART('D');
                    prdbg('=', command);
                    checksum = crc8(checksum^command);
                    datacount = commands[command].bytestoreadin;

                    if(datacount == 0){ //Special case of pure commands (getdips etc)
	                    //Will call the function to generate data whilst the clock stretch
	                    //is in effect
	                    mputcharUSART('E');
                        state = WAIT;
                        //break;
                    }else
                    state=GOTCOMMAND;
                
                    break;

                case GOTCOMMAND:              
                    //Read in the data   
                    //mputcharUSART('F');    
                    tmpdata = SSPBUF;
                     i2c_debug prdbg('<', tmpdata);
        			PIR1bits.SSPIF = 0;
                    data[datapos] = tmpdata; // start entering data at start of array
                    checksum = crc8(checksum^data[datapos]);
                    datapos++;
                    if(datapos == datacount)
                        state = GOTDATA;
                    break;

                case GOTDATA:
                	mputcharUSART('G');
                	tmpdata = SSPBUF;
                	i2c_debug prdbg('?', tmpdata);
        			PIR1bits.SSPIF = 0;
                    //if (tmpdata == checksum) 
                    if (1) // temporary fix so no checksumm for testing
	                {
                        commands[command].docmd(data);
                        i2cstatus = GOOD; 
                    }
                    else i2cstatus = BAD;
                    state = WAIT;
                    break;
             	case GOTADDRESSWRITE:
             		mputcharUSART('H');

        			PIR1bits.SSPIF = 0;
             		if (datapos<datacount)
             		{
	             		mputcharUSART('P');
	             		//mputcharUSART('a'+datapos);
	             		//mputcharUSART('A' + data[datapos]);
	             		SSPBUF = data[datapos];
	             		i2c_debug prdbg(']', data[datapos]);
	             		SSPCON1bits.CKP = 1;
	             		checksum = crc8(checksum^data[datapos]);
	             		datapos++;
	             	}
	             	else
	             	{
		             	mputcharUSART('I');

		             	SSPBUF = checksum;
		             	i2c_debug prdbg('*', checksum);
		             	SSPCON1bits.CKP = 1;
		             	state = SENTCHECKSUM;
		         	}
             		break;
             	case SENTCHECKSUM:
             		mputcharUSART('J');
             		i2cstatus = GOOD;
             		PIR1bits.SSPIF = 0;
             		break;
             	default:
             		tmpdata = SSPBUF;
             	i2c_debug 	prdbg('!', tmpdata);
		            
             		
            }
            
            //mputcharUSART('O');
        }
    }
    return;
}

void identify(u8 *data){
	data[0]='I'; // maby read from EE later?
	return;
	}	
void setled(u8 *data){
    //PORTD = (*data << 4)&0xf0;
    PORTD = PORTD |( 0x70&((*data << 4)&0xf0));
    return;
}

void checkusb(u8 *data){
	data[0]=usbflag;
	mputcharUSART('Q');
	//read bit = <6> i2c -> usb
	//write bit  = <7> usb ->i
	// 5 lsb position within sector
	
	return;
	}
void getv(u8 *data){
	data[0]= (u8)(voltage&0x00FF);
	data[1]= (u8)((voltage&0xFF00)>>8);
	return;
	}
void geti(u8 *data){
	data[0]= (u8)(current&0x00FF);
	data[1]= (u8)((current&0xFF00)>>8);
	return;
	}
void getdip(u8 *data){
	data[0]=PORTD&0x0F;
	return;
	}
	
void setrails(u8 *data){
	PORTE= data[0]&0x07;
	PORTB= (PORTB&(~0x18))|((data[0]>>1)&0x18);
	return;
	}
void getrails(u8 *data){
	data[0]= (PORTE&0x07)|((PORTB&0x18)<<1);
	return;
	}
void getusbbuf(u8 *data)
{
	unsigned char pos;
	
	for(pos=0; pos<32; pos++)
		data[pos] = sd_outbuf[pos];	
}
void setusbbuf(u8 *data)
{
	mputcharUSART('S');
	usbflag=0;
}		

void sendser(u8 *data){
	//char strcount =0; // send string to ring buffer untill null.
	//while(data[strcount]!= 0 ) mputcharUSART(data[(strcount++)]); 
	//mputcharUSART(data[1]);
	mputcharUSART(data[0]);
	
	return;
	}
	
void getsectorlo(u8 *data)
{
	data[1]=(u8)((sectadd>>8)&0xff);
	data[0]=(u8)(sectadd&0xFF);
	mputcharUSART('F');
}
void getsectorhi(u8 *data)
{
	data[1]=(u8)((sectadd>>24)&0xff);
	data[0]=(u8)((sectadd>>16)&0xFF);
}

void datagood(u8 *data)
{
		usbflag=0;
}
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

    //rd0-3 switch
    //rd4-7 leds
    //re0 - big power motor fet thing
    //e1 - fet control servo rail
    //re2 - fet slug rail

    PORTC=0x01; // MUST BE set BEFORE UNTRISTATING ELSE SLUG BOOT!!!
    TRISA=0XFF;
    TRISB=0XFF;
    TRISC=0XFE;// make slug pin Out
    TRISD=0x0F;
    TRISE = 0;
    PORTE = 0b110; // turn all power rails on
    //IN real life will be 111 but changted to accomodate prototype2 relay error

    
    PORTD =0;
	

    delay(50);
    PORTCbits.RC0=0;// blip slug
    delay(20);
    PORTCbits.RC0=1; // never press the button, ever!! (dont hold down)
    delay(20);

    // end tempfw inserts
    
    init_usart();

    TRISC|=0x20;
    OpenUSART(
            USART_TX_INT_OFF &
            USART_RX_INT_OFF &
            USART_ASYNCH_MODE &
            USART_EIGHT_BIT &
            USART_CONT_RX & 
            USART_BRGH_HIGH, // checked, this does mean bit brgh bit set
            152);  // this is 19200


    //-------ADC setup ---------------------------	

    OpenADC( ADC_FOSC_64 &
            ADC_RIGHT_JUST &
            ADC_20_TAD,

            ADC_CH0 &
            ADC_INT_OFF &
            ADC_VREFPLUS_VDD &
            ADC_VREFMINUS_VSS
            , 0x0D );// this is to A/D pins ref from rails
            
            // have to start the viscious circle
            SetChanADC(ADC_CH0);
			ConvertADC();

    //----I2C setup ---------

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

    //SSPADD=0x55<<1; //55 used in slug software
    SSPADD=0x3f<<1;
    //	clear sm flags;....
    
    //SSPCON1bits.SSPOV; //Receive overflow indicator

    // -------current sence set up---------------------

    TRISBbits.TRISB3=0;
    PORTBbits.RB3 = 1; // !shdn
    PORTBbits.RB2 = 0;    	// gsel 0 allows upto appx 4A with track resistor (heating element hot!)
    // gesl 1 allows upto about 2A ish		





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



void swin(void)
{
    char strcount;
    if (PORTD&0x01)// switch nearest usb socket
    {
    }

    return(0);
}

#define POLY    (0x1070U << 3)

u8 crc8(u8 tempdata){
    int i;
    int data;
    data = (u16)tempdata<<8;
    for(i = 0; i < 8; i++) {
        if (data & 0x8000)
            data = data ^ POLY;
        data = data << 1;
    }
    return (u8)(data >> 8);
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

void prdbg(char sentinel, char data)
{
	char i=0,	b[4];
	mputcharUSART(sentinel);
	btoa(data,b);
    while(b[i]!= 0 ) mputcharUSART(b[(i++)]); 
}


/** EOF main.c ***************************************************************/



