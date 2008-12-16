#include"device.h"
#include "usart.h"
#include <signal.h>
#include "timed.h"

#define HOWSAFE 17 		/* I guess these should maby live somewhere else? */

const uint8_t safe[HOWSAFE]={
	0x7E, //  framing byte
	0, 0x0D, // length
	0x80,             // API identifier for ive recived a packet
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, //# Source address -- this should be ignoored
	0, //rssi -ignoored
	0xff, // options -- ignoored
	0x01,0x02, // the actuall data!!!
	0x00};//last one is checksum

interrupt (USART1TX_VECTOR) uart_tx_isr(void)
{
	U1TXBUF = 0xAA;		/* transmit to calibrate on oscope */
}

/* --------------receive---------------------- */


interrupt (USART1RX_VECTOR) uart_rx_isr(void)
{
	uint8_t u1rxbuf_l = 0;
	u1rxbuf_l =U1RXBUF;
	xbee_handler(u1rxbuf_l);
}



void usart_init(void) {
	P3SEL |= (1 << 6) | (1 << 7);	// P3 bits 6,7 used by uart
	P3DIR |= (1 << 6);		// P3 bit 6 tx
	P3DIR &= ~(1 << 7);		// P3 bit 7 rx

	U1CTL = SWRST + CHAR;
	U1TCTL = SSEL_SMCLK;	/* Aclk 8MHz */
	U1RCTL = 0;

	U1BR0 = 0x45;		/* baud hi and lo */
	U1BR1 = 0x00;
	U1MCTL = 0xAA;		/* modulation factor */



	ME2 |= UTXE1;	// enable transmitter
	ME2 |= URXE1;	// enable receiver

	U1CTL &= ~SWRST;
	
	IE2 |= UTXIE1|URXIE1;



}


void xbee_handler(uint8_t rxbuf)
{
	static uint8_t place=0;
	static uint8_t xbchecksum=0;
	static uint8_t delim=0;

	    
	if(rxbuf==0x7e )//is it a sentinel if so resart
	{
		place=1;
	       	xbchecksum=0;
		return;
	}
			
			
	/* escaped char replacemnt scheme: */
	if(delim)		/* previous byte was delimiter */
	{
		rxbuf ^= 0x20;
		delim =0;
	}

	if(rxbuf==0x7D)
	{
		delim =1;	/* next byte needs to be doctored */
		return;
	}
			

	/* is value within the areas we care about, if so, is it the same as the buffer */
	if(((place>STARTIG)&&(place<STOPIG))||(safe[place]==rxbuf)||(place>HOWSAFE-2))//
	{ //      in the ignoor zone                 char corret           checksum???
		/* temp[place]=rxbuf; */
		if (place==HOWSAFE-1) /* reached end of string */
		{		       
			if(rxbuf+xbchecksum==0xff)
			{
				/* the packet is verifyed */
				make_safe();
			}
			else 
			{
				place=0;
						   	
			}
		}
		else
		{
			if ((place>STARTIG)&&(place<HOWSAFE-1))/*  add up bytes between end of length and penultimate(checksum) */
			{
				xbchecksum+=rxbuf;
			}
			place++;
		}
	} 
	else place=0;		/* byte was not in the expected series - fail */
	
    
    
}


/* static inline void putc(char c) { */
/* 	while(!(IFG2 & UTXIFG1));	// wait for tx buf empty */
/* 	U1TXBUF = c; */
/* } */

/* static inline unsigned int getc(void) { */
/* 	while(!(IFG2 & URXIFG1));	// wait for rx buf full */
/* 	return U1RXBUF; */
/* } */
