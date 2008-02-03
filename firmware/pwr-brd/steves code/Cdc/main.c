// LD_LIBRARY_PATH=/usr/local/lib xbd -s /dev/ttyS0 --init-baud 9600 -b 57600
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

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include "system\typedefs.h"                        // Required
#include "system\usb\usb.h"                         // Required
#include "io_cfg.h"                                 // Required

#include "system\usb\usb_compile_time_validation.h" // Optional
//#include "user\user.h"                              // Modifiable

#include <i2c.h>
#include <usart.h>
#include "musart.h"
#include <stdlib.h>
#include <adc.h>
#include <timers.h>

#define i2c_debug if (0)

#define GOOD 0
#define BAD 1
#define u8 unsigned char
#define u16 unsigned int

//saftey system constants
#define HOWSAFE 17 // zero indexed elsewhere but this is the total no....

// values for range within which to ignoor data - source address,rss and optoins seciotn
#define STARTIG 2
#define STOPIG 14 // this prob wont need to change with packet loength. 2-8 just skips address to rssi incl.



/** V A R I A B L E S ********************************************************/

#pragma udata
int alive;
long int startupdel;



u8 data[32]; // size according to smbus spec 
#pragma udata


u8 temp[HOWSAFE];


int bcount;

int voltage = 0x5555;// local variables holding results of adc
int current = 0xAAAA;

unsigned long sectadd=0xabcdef12;
unsigned char usbflag=0x00; // non zero means usb i2c bridge needs serviceing , maby use to give idea of direction etc. 
unsigned char i2cstatus = BAD;


typedef struct {
    u8 bytestoreadin;
    u8 bytestoreadout;
    void (*docmd)(u8 *data);
} t_command;


/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void USBTasks(void);
void delay(int time);
void prdbg(char sentinel, char data);

void i2cservice(void);
u8 crc8(u8 tmpdata);
void usartrxservice(void);

void low_isr(void);
void high_isr(void);

void adcserv(void);



void identify(u8 *data);
void setled(u8 *data);
void checkusb(u8 *data);
void getv(u8 *data);
void geti(u8 *data);
void getdip(u8 *data);
void setrails(u8 *data);
void getrails(u8 *data);
void sendser(u8 *data);
//void getusbbuf(u8 *data);
//void setusbbuf(u8 *data);
//void getsectorlo(u8 *data);
//void getsectorhi(u8 *data);
//void datagood(u8 *data);



//#pragma romdata // this seems to do squat all!
//						{bytes in, bytesout, function name}
 t_command commands[] = {{0, 4,identify}, //0
                        {1, 0,setled},
                        {0, 1,checkusb},//2
                        {0, 2,getv},
                        {0, 2,geti},//4
                        {0, 1,getdip},
                        {1, 0,setrails},//6
	                    {0, 1,getrails},
	                    {1,1,sendser}};//8
							
							
//----------------------
		                //{0,32,getusbbuf},//8
			            //{32,0,setusbbuf},
			            //{0,2,getsectorlo},
				        //{0,2,getsectorhi},//10
					    //{32,0,setusbbuf},//old not really needed kept for 
						//{1, 0,datagood}

#pragma udata

//0x7e 0x0 0xb 0x81 0x0 0x0 0x3b 0x2 0x0 0xff 0x1 0x2 0x3 0x4 0x38

#pragma romdata


//THIS IS THE ALIVE PACKET. IF ANYONE CHANGES IT, THEY WILL BE ILL WITH A THOUSAND PLAGUES.
// NB: it is the packet Dave(Phil) and Mr Rob have termed the "Ping packet" 
unsigned char safe[HOWSAFE]={
	  0x7E, //  framing byte
	  0, 0x0D, // length
      0x80,             // API identifier for ive recived a packet
      0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, //# Source address -- this should be ignoored
      0, //rssi -ignoored
      0xff, // options -- ignoored
      0x01,0x02, // the actuall data!!!
      0x00};//last one is checksum

/*
unsigned char safe[HOWSAFE]={
	  0x7E, 0, 0x0b,
      0x80,                     				// API identifier
      0x00,0x01,0x02,0x03, //# Source address
      0,0xff,0x01,0x02,0x03,0x04,0x00};//last one is checksum
*/
/*	
unsigned char safe[HOWSAFE]={
	  0x7E, 0, 17,
      0x80,                     				// API identifier
      0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, //# Source address
      0x54,                                    //# RSSI
      0,                                       //# Options
      0,0xff,0x01,0x02,0x03,0x04,0x00};//last one is checksum
  */    
      
#pragma udata 

/** V E C T O R  R E M A P P I N G *******************************************/

extern void _startup (void);        // See c018i.c in your C18 compiler dir
#pragma code _RESET_INTERRUPT_VECTOR = 0x000800
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

/*
* For PIC18 devices the low interrupt vector is found at
* 00000018h. The following code will branch to the
* low_interrupt_service_routine function to handle
* interrupts that occur at the low vector.
*/
#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
_asm GOTO low_isr _endasm
}
#pragma code /* return to the default code section */


#pragma interruptlow low_isr
void low_isr (void)
{
//	PORTD|=0b01000000;
	//INTCONbits.TMR0IF=0;
}




/*
* For PIC18 devices the high interrupt vector is found at
* 00000008h. The following code will branch to the
* high_interrupt_service_routine function to handle
* interrupts that occur at the high vector.
*/
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
_asm GOTO high_isr _endasm
}
#pragma code /* return to the default code section */


#pragma interrupt high_isr
void high_isr (void)
{
	if (PORTDbits.RD0) // check for test mode
	{
		PORTE|=0b00000001;
	}
	else				// no so drop out because in cometition and theres no ping packet to reset the counter and its overflowed
	{	
		//PORTD^=0b00010000;
		PORTE&=0b11111110;
	}
	INTCONbits.TMR0IF=0;
}

#pragma code





void main(void)
{
	unsigned char spoof ='A';


	
    InitializeSystem();
    
    
    //baud rate generator
    
    while(1)
    {
		    //PORTD|=0b01000000;
	    
	    /*if (alive++==8000)
	    {
		    alive=0;
		    //PORTDBits.RD7=!PORTDBits.RD7;
		    PORTD^=0b10000000;
		   
		}*/

		if (PORTDbits.RD0) // check for test mode
		{
			PORTE|=0b00000001;
		}	

		if(PORTAbits.RA4) USBTasks();         // USB Tasks
	    //if(mUSBUSARTIsTxTrfReady()) mUSBUSARTTxRam( &spoof, 1);
        i2cservice();
        usartrxservice();
		adcserv();  // this may need to be moved intot he slow alive loop as it caused problems in the past
        //ProcessIO();        // See user\user.c & .h
    }//end while
}//end main




void usartrxservice(void)
{
static u8 place=0;
static u8 xbchecksum=0;
static u8 delim=0;

u8 rxbuf;
//u8 temp;
    
    if(PIR1bits.RCIF)
    {
	  
	    //buffer value so can use
	    rxbuf=RCREG;
	    
	    
		    if(rxbuf==0x7e )//is it a sentinel if so resart
		    {
			    place=1;
			    xbchecksum=0;
			    return;
			}
			
			
			//escaped char replacemnt scheme:
			if(delim)
			{
				rxbuf ^= 0x20;
				//PORTD^=0b01000000;
				delim =0;
			}
			if(rxbuf==0x7D)
			{
				delim =1;
				return;
			}
			
		    // is value within the areas we car about, if so, is it the same as the buffer
		    if(((place>STARTIG)&&(place<STOPIG))||(safe[place]==rxbuf)||(place>HOWSAFE-2))//
			{ //      in the ignoor zone                 char corret           checksum???
				temp[place]=rxbuf;
				    if (place==HOWSAFE-1) 
				    {
					    //temp = rxbuf+xbchecksum;
					    //prdbg('A', temp);
					    //if(mUSBUSARTIsTxTrfReady()) mUSBUSARTTxRam( &temp , 1);
					    if(rxbuf+xbchecksum==0xff)
					    {
					    	place=TMR0L=TMR0H=0; // we have got the the end of the buffer so restart he timer
					   		//PORTD^=0b00100000;// debug lights
					   		PORTE|=0b1;
					   	}
					   	else 
					   	{
						   	place=0;
						   	
						}
					}
					else
					{
						if ((place>STARTIG)&&(place<HOWSAFE-1))// add up bytes between end of length and penultimate(checksum)
						{
							xbchecksum+=rxbuf;
						}
						place++;
					}
				} 
			else place=0;//byte was wrong
	}
    
    
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
  /*
    PORTE = 0b000;
    PORTE = 0b001;
    PORTE = 0b010;
    PORTE = 0b100;
    PORTE = 0b010;
    PORTE = 0b011;
    PORTE = 0b011;
    i and steve think issues from reading from porte eg porte|=0x32;
    
*/

    delay(25);
    PORTE = 0b111; 
    delay(25);
    PORTCbits.RC0=0;// blip slug
    delay(5);
    PORTCbits.RC0=1; // never press the button, ever!! (dont hold down)
    delay(5); // JUST TO BE SURE NO POWER RAIL FLUCTUATION
    
    
    
    //init_usart();

    TRISC|=0x20;
    OpenUSART(
            USART_TX_INT_OFF &
            USART_RX_INT_OFF &
            USART_ASYNCH_MODE &
            USART_EIGHT_BIT &
            USART_CONT_RX & 
            USART_BRGH_HIGH, // checked, this does mean bit brgh bit set
            207);
            BAUDCONbits.BRG16=1;
            ////25); // this it 115200
            ////152);  // this is 19200
            // 57600 - 207 brg and brg16 set
         
            //1249 - 9600 baug brg and brg16 set


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
	TRISBbits.TRISB2=0;
    PORTBbits.RB3 = 1; // !shdn
    PORTBbits.RB2 = 0;    	// gsel 0 allows upto appx 4A with track resistor (heating element hot!)
    // gesl 1 allows upto about 2A ish		

    // ADCON1 |= 0x0F;                 // Default all pins to digital

	//configure timer0
	OpenTimer0(T0_16BIT& T0_SOURCE_INT&T0_PS_1_128&TIMER_INT_ON);
	// 12mips, 128 prescale 16bit overflow = 1.43s ((((48000000)/4)/128)/(2^16) = 1.43...  
	
	//T0CON =0b10000000;
	//TMR0L=0;
	//TMR0H=0;
	
	//configure interrupts

	//INTCON=0b00100111;
	INTCON=0b00100000;
	INTCON2=0b00000100;
	INTCON3=0;
	PIE1=0;
	PIE2=0;
	IPR1=0;
	IPR2=0;
	INTCONbits.GIE=1;

	


#if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See io_cfg.h
#endif

#if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;
#endif
    mInitializeUSBDriver();         // See usbdrv.h
    
    //taken out from steves - check this
    // UserInit();                     // See user.c & .h

}//end InitializeSystem



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
	data[0]='P'; // maby read from EE later?
	data[1]='B'; 
	data[2]=0; 
	data[3]=1; 
	return;
	}	
void setled(u8 *data){
    //PORTD = (*data << 4)&0xf0;
    PORTD = PORTD |( 0x70&((*data << 4)&0xf0));
    //if(mUSBUSARTIsTxTrfReady()) mUSBUSARTTxRam( &data[0], 1);
    return;
}

void checkusb(u8 *data){
	data[0]=usbflag;
	//mputcharUSART('Q');
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

void sendser(u8 *data){
	//PORTD|=0x00100000;
	if(mUSBUSARTIsTxTrfReady()){
		mUSBUSARTTxRam( &data[0], 1);	
		data[0]=0; //return success
		}
	else{
		data[0] = 1; // return failure
	}

	return;
	}

/*	
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
*/



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
    
    #if defined(USB_USE_CDC)
    CDCTxService();
    #endif

}// end USBTasks






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


void adcserv(void)
{
	if (!BusyADC())
	{
				    
		// currently this totallyt rodgers the msd code, dunno why!
		
	
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

void prdbg(char sentinel, char data)
{
	/*
	char i=0,	b[4];
	mputcharUSART(sentinel);
	btoa(data,b);
    while(b[i]!= 0 ) putsUSBUSART(b[(i++)]); 
    //mputcharUSART(b[(i++)]); 
    */
}



/** EOF main.c ***************************************************************/
