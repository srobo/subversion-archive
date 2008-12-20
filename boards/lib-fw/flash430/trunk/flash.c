#include "flash.h"
#include "dev.h"
#include <signal.h>

/* Values of the registers */
#define FCTL1_VAL 0
		/* BLKWRT = 0: Byte/word write mode (not block) */
		/* WRT = 0: Not in write mode at the moment */
		/* EEIEX  = 0: No interrupts during write */
		/* EEI = 0: No interrupts during erase */
		/* MERAS:ERASE = 0: No erase. */;

#define FCTL2_VAL (FSSEL_SMCLK | 43)
		/* Flash controller clocked by SMCLK */
		/* Divide the 16 MHz DCO to 363 KHz: */

#define FCTL3_VAL 0

/* Lock the flash against writes and erasure */
#define flash_lock() do { FCTL3 = FCTL3_VAL | FWKEY | LOCK; } while (0)
#define flash_unlock() do { FCTL3 = FCTL3_VAL | FWKEY; } while (0)
#define flash_busy() ( FCTL3 & BUSY )

#define flash_write_mode() do { FCTL1 = FCTL1_VAL | FWKEY | WRT; } while (0)
#define flash_write_mode_off() do { FCTL1 = FCTL1_VAL | FWKEY; } while (0)

/* Get a pointer to the segment that x is in */
#define mem_segment(x) ( (uint16_t*)(((uint16_t)x) & 0xfe00 ) )

/* The address of the next chunk */
uint16_t *next_chunk;

/* Pointer to the last area to be erased */
static uint16_t *last_erased;

/* Buffer for the interrupt vector table */
static uint16_t ivt_buf[32];

bool firmware_received;

void flash_write_chunk( const uint16_t *source, uint16_t *dest )
{
	uint8_t i;

	/* Check that the section has been erased */
	if( last_erased < mem_segment( dest ) )
		flash_erase_segment( dest );

	flash_unlock();
	flash_write_mode();
	/* Now write it */
	for( i=0; i<(CHUNK_SIZE/2); i++ )
		*(dest + i) = *(source + i);

	flash_write_mode_off();
	flash_lock();
}

void flash_init( void )
{
	/* Pointer into the other area */
	uint16_t *other_area;

	FCTL1 = FCTL1_VAL | FWKEY;
	FCTL2 = FCTL2_VAL | FWKEY;

	flash_lock();

	/* Calculate which area we need to write to */
	if( (uint16_t)mem_segment( flash_init ) >= FLASH_AREA_1 )
		other_area = (uint16_t*)FLASH_AREA_0;
	else
		other_area = (uint16_t*)FLASH_AREA_1;

	/* Nothing's been written yet */
	next_chunk = other_area;

	firmware_received = FALSE;

	last_erased = mem_segment(other_area - 1);
}

void flash_erase_segment( uint16_t *addr )
{
	flash_unlock();

	/* Segment erase -> MERAS = 0, ERASE = 1 */
	FCTL1 = (FCTL1 & (~FRKEY)) | FWKEY | ERASE;

	/* Initiate the erase with a dummy write */
	*addr = 0xffff;

	flash_lock();

	last_erased = mem_segment(addr);
}

void flash_rx_chunk( uint16_t c_addr, const uint16_t *fw)
{
	uint8_t i;
	/* Pointer to the chunk */
	uint16_t *c = (uint16_t*)c_addr;

	/* Only accept 16-byte aligned chunks */
	if( c_addr % 16 != 0 )
		return;

	if( c >= IVT )
	{
		/* It's an interrupt vector table entry */
		uint8_t cpos = c - IVT;

		for( i=0; i<(CHUNK_SIZE/2); i++ )
			ivt_buf[i + cpos] = fw[i];

		/* Last entry? */
		if( c == IVT + 24 )
		{
			/* Got the IVT */
			firmware_received = TRUE;

			next_chunk = 0;
		}
		else
			next_chunk = c + CHUNK_SIZE/2;
	}
	else if( c == next_chunk )
	{
		flash_write_chunk( fw, c );

		next_chunk += CHUNK_SIZE/2;
	}
}

void flash_switchover( void )
{
	uint8_t i;

	if( !firmware_received )
		return;

	flash_write_mode_off();

	/* Disable interrupts */
	dint();

	for( i=0; i<32; i += (CHUNK_SIZE/2) )
		flash_write_chunk( ivt_buf + i, IVT + i );

	/* Finished loading new firmware! */
	/* Jump to the reset vector! */
	(*(void (*)()) (ivt_buf[31])) ();
}
