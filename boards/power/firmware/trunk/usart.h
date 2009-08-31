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

void usart_init(void);

void rx_byte(uint8_t rxbuf);

#endif	/* __USART_H */
