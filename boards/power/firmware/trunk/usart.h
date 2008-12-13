#ifndef __LED_H
#define __LED_H
#include <stdint.h>
void usart_init(void);
/* static inline void putc(char c); */
/* static inline unsigned int getc(void); */

void xbee_handler(uint8_t rxbuf);
/* values for range within which to ignoor data - source address,rss and optoins section */
#define STARTIG 2
#define STOPIG 14 // this prob wont need to change with packet length. 2-8 just skips address to rssi incl.

/* THIS IS THE ALIVE PACKET. IF ANYONE CHANGES IT, THEY WILL BE ILL WITH A THOUSAND PLAGUES. */
/* NB: it is the packet Dave(Phil) and Mr Rob have termed the "Ping packet"  */

#define HOWSAFE 17 		/* I guess these should maby live somewhere else? */

static uint8_t safe[HOWSAFE]={
	0x7E, //  framing byte
	0, 0x0D, // length
	0x80,             // API identifier for ive recived a packet
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, //# Source address -- this should be ignoored
	0, //rssi -ignoored
	0xff, // options -- ignoored
	0x01,0x02, // the actuall data!!!
	0x00};//last one is checksum


#endif
