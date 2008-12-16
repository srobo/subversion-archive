#ifndef __USART_H
#define __USART_H
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




#endif
