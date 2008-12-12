#include"device.h"
#include "usart.h"
#include <signal.h>
/* ----------------- transmit-------------------- */

interrupt (USART1TX_VECTOR) uart_tx_isr(void)
{
	U1TXBUF = 'P';
}

/* --------------receive---------------------- */


interrupt (USART1RX_VECTOR) uart_rx_isr(void)
{
	uint8_t temp =0;
	static enum states {
		idle,
		w4adhi,
		w4adlo,
		w4data,
		w4cksum
	} state = idle;

	state = idle;
	temp = U1RXBUF;

}



void usart_init(void) {
	U1CTL = SWRST + CHAR;
	U1TCTL = SSEL_ACLK;	/* Aclk 8MHz */
	U1RCTL = 0;


	U1BR0 = 0x41;	// 9600, 8MHz clock
	U1BR1 = 0x3;
	U1MCTL = 0x9;


/* 	U1BR0 = 0xF0; */
/*  	U1BR1 = 0x13; */
/*  	U1MCTL = 0x04; */


	
	//ME2 |= UTXE1;	// enable transmitter
	ME2 |= URXE1;	// enable receiver

	U1CTL &= ~SWRST;
	
	IE2 |= UTXIE1|URXIE1;

	P3SEL |= (1 << 6) | (1 << 7);	// P3 bits 6,7 used by uart
	P3DIR |= (1 << 6);		// P3 bit 6 tx
	P3DIR &= ~(1 << 7);		// P3 bit 7 rx

}

static inline void putc(char c) {
	while(!(IFG2 & UTXIFG1));	// wait for tx buf empty
	U1TXBUF = c;
}

static inline unsigned int getc(void) {
	while(!(IFG2 & URXIFG1));	// wait for rx buf full
	return U1RXBUF;
}

