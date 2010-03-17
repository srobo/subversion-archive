#include"device.h"
#include "usart.h"
#include <signal.h>
#include "timed.h"
#include "led.h"

static const xb_alive_t safe[] =
{
	{ 0x7E, XB_NO_CHKSUM },

	/* Length: */
	{ 0x00, XB_NO_CHKSUM },
	{ 0x0D, XB_NO_CHKSUM },

	/** Frame data **/
	/* API code for 64-bit frame received */
	{ 0x80, 0 },

	/* Address */
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },
	{ 0x00, XB_DONT_CARE },

	/* RSSI */
	{ 0x00, XB_DONT_CARE },
	/* Options */
	{ 0x00, XB_DONT_CARE },

	/* Frame data */
	{ 0x01, 0 },
	{ 0x02, 0 },

	/* Checksum not included here -- do that in code */

	{ 0, XB_END }
};

interrupt (USART1TX_VECTOR) uart_tx_isr(void)
{
	U1TXBUF = 0xAA;		/* transmit to calibrate on oscope */
}

interrupt (USART1RX_VECTOR) uart_rx_isr(void)
{
	uint8_t u1rxbuf_l = 0;

	u1rxbuf_l =U1RXBUF;
	rx_byte(u1rxbuf_l);
}

void usart_init(void) {
	P3SEL |= (1 << 6) | (1 << 7);	// P3 bits 6,7 used by uart
	P3DIR |= (1 << 6);		// P3 bit 6 tx
	P3DIR &= ~(1 << 7);		// P3 bit 7 rx

	U1CTL = SWRST + CHAR;
	U1TCTL = SSEL_SMCLK;	/* Aclk 8MHz */
	U1RCTL = 0;

	/* set for 57800 */
	U1BR0 = 0x8a;		/* baud hi and lo */
	U1BR1 = 0x00;
	U1MCTL = 0xef;		/* modulation factor */

	ME2 |= UTXE1;	// enable transmitter
	ME2 |= URXE1;	// enable receiver

	U1CTL &= ~SWRST;
	
	IE2 |= UTXIE1|URXIE1;
}

void rx_byte( uint8_t b )
{
	static uint8_t pos = 0;
	static uint8_t checksum = 0;
	static uint8_t escape = 0;

	if( b == 0x7E ) {
		escape = 0;
		checksum = 0;
	}

	if( escape ) {
		b ^= 0x20;
		escape = 0;
	} else if( b == 0x7D ) {
		escape = 1;
		return;
	}

	/* Reached the end? */
	if( safe[pos].flags & XB_END ) {
		/* Evaluate checksum and mark as valid */
		if( checksum + b == 0xff )
			reset_cutoff();

		pos = 0;
		return;
	}

	if( !(safe[pos].flags & XB_NO_CHKSUM) )
		checksum += b;

	if( !(safe[pos].flags & XB_DONT_CARE)
	    && safe[pos].val != b ) {
		pos = 0;
		return;
	}

	pos++;
}

/* static inline void putc(char c) { */
/* 	while(!(IFG2 & UTXIFG1));	// wait for tx buf empty */
/* 	U1TXBUF = c; */
/* } */

/* static inline unsigned int getc(void) { */
/* 	while(!(IFG2 & URXIFG1));	// wait for rx buf full */
/* 	return U1RXBUF; */
/* } */
