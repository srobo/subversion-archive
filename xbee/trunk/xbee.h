#ifndef __XBEE_H
#define __XBEE_H
#define _GNU_SOURCE		/* For TEMP_FAILURE_RETRY */
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>
#include <glib.h>

#define XB_INBUF_LEN 512
#define XB_OUTBUF_LEN 512

typedef struct
{
	uint16_t len;
	uint8_t *data;
} xb_frame_t;

typedef struct
{
	int fd;

	/* Whether we're in API mode */
	gboolean api_mode;
	
	/* Whether we're in AT command mode */
	gboolean at_mode;
	/* The time at which we entered AT command mode */
	struct timeval at_time;

	/*** Transmission ***/
	/* Queue of outgoing frames - all of xb_frame_t */
	/* Note: the data in these frames has already been escaped */
	GQueue* out_frames; 

	/* Checksum value for the currently transmitting frame */
	uint8_t o_chk;
	gboolean checked;	/* Whether the checksum has been calculated */
	gboolean tx_escaped;	/* Whether the next byte has been escaped */
	/* The next byte to be transmitted within the current frame */
	uint16_t tx_pos;

	/*** Reception ***/
	/* Buffer of incoming data */
	/* Always contains the beginning part of a frame */
	uint8_t inbuf[XB_INBUF_LEN];
	/* The number of bytes in the input buffer */
	uint16_t in_len;
	/* Whether the next byte received should be escaped */
	gboolean escape;

	/*** Stats ***/
	uint32_t bytes_discarded, frames_discarded; /* Frames with invalid checksums */
	uint32_t bytes_rx, bytes_tx;   
	uint32_t frames_rx, frames_tx;  /* Valid checksum frames received */

} xbee_t;

typedef enum
{
	XB_ADDR_16,
	XB_ADDR_64
} xb_addr_len_t;

typedef struct
{
	xb_addr_len_t type;
	uint8_t addr[8];
} xb_addr_t;

/* static inline uint16_t XB_ADDR_GET_16(xb_addr_t* addr) */
/* { */
/* 	assert(addr != NULL && addr->type == XB_ADDR_16 ); */
/* 	return (uint16_t)(addr->addr & 0xFFFF); */
/* } */

/* static inline uint64_t XB_ADDR_GET_64(xb_addr_t* addr) */
/* { */
/* 	assert(addr != NULL && addr->type == XB_ADDR_64 ); */
/* 	return addr->addr; */
/* } */

/* Initialise the module.
 * This puts the node into API mode.
 * xb: The xbee structure.
 * fd: serial port file descriptor. */
void xbee_init( xbee_t* xb, int fd );

gboolean xbee_write( xbee_t* xb, void* buf, uint16_t len );

gboolean xbee_main( xbee_t* xb );

#endif	/* __XBEE_H */
