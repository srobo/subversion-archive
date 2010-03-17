#ifndef __USART_H
#define __USART_H
#include <stdint.h>

enum {
	/* We don't care about the value of this byte */
	XB_DONT_CARE = 1 << 0,
	/* Don't include this byte in the checksum */
	XB_NO_CHKSUM = 1 << 1,

	/* Last byte */
	XB_END = 1 << 2
};

typedef struct {
	uint8_t val;
	/* Bitmask of the flags from the above enum */
	uint8_t flags;
} xb_alive_t;

void usart_init(void);

#endif	/* __USART_H */
