/*   Copyright (C) 2008 Robert Spanton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
#include "msp430-fw.h"
#include "sr-i2c.h"
#include "i2c-blk.h"

/* Number of times to retry 'calling' the device */
#define MSP430_FW_RETRIES 10

uint8_t commands[NUM_COMMANDS];
uint8_t* msp430_fw_i2c_address = NULL;
uint16_t msp430_fw_bottom = 0;
uint16_t msp430_fw_top = 0;

static void graph( char* str, uint16_t done, uint16_t total );

gboolean msp430_get_fw_version( int fd, uint16_t *ver, gboolean give_up )
{
	uint16_t c = 0;
	g_assert( ver != NULL );

	while( sr_i2c_read_word( fd, commands[CMD_FW_VER], ver ) < 0 ) {

		if ( give_up ) {
			if( c > MSP430_FW_RETRIES )
				return FALSE;

			/* Spend a little more time when in "give-up" mode */
			/* Who knows, maybe some noise will pass... */
			usleep(20000);
			c++;
		} else
			g_print( "Failed to read firmware version... retrying.\n" );
	}

	return TRUE;
}

uint16_t msp430_get_next_address( int fd )
{
	uint16_t r1, r2;

	do {
		r1 = msp430_get_next_address_once(fd);
		r2 = msp430_get_next_address_once(fd);
	} while ( r1 != r2 );

	return r1;
}

void msp430_send_block( int fd, 
			uint16_t fw_ver,
			uint16_t addr,
			uint8_t *chunk )
{
	uint8_t b[4 + CHUNK_SIZE];

	/* Format:
	   0-1: Firmware version (1 is lsb)
	   2-3: Address (3 is lsb)
	   4-19: The data */

	b[0] = fw_ver & 0xff;
	b[1] = (fw_ver >> 8) & 0xff;
	b[2] = addr & 0xff;
	b[3] = (addr >> 8) & 0xff;

	g_memmove( b + 4, chunk, CHUNK_SIZE );

	if( sr_i2c_block_write( fd, commands[CMD_FW_CHUNK], CHUNK_SIZE + 4, b, *msp430_fw_i2c_address ) < 0 )
		g_error( "Failed to write data" );
}

uint16_t msp430_get_next_address_once( int fd )
{
	uint16_t r;

	while( sr_i2c_read_word( fd, commands[CMD_FW_NEXT], &r ) < 0 );

	return r;
}

void msp430_send_section( int i2c_fd,
			  elf_section_t *section, 
			  gboolean check_first )
{
	uint16_t next;
	g_assert( section != NULL );

	if( check_first ) {
		next = msp430_get_next_address( i2c_fd );

		if( next != section->addr )
			g_error( "I've got the wrong binary -- need one that starts at %hx, got %hx\n", next, section->addr );
	}
	else
		next = section->addr;

	printf( " " );

	while( next < (section->addr + section->len) 
	       /* MSP430 indicates all firmware received with 0 */
	       && next != 0 ) {
		uint16_t rem, done;
		uint8_t *chunk;

		/* Must be CHUNK_SIZE aligned */
		g_assert( next % CHUNK_SIZE == 0 );
		g_assert( next >= section->addr );

		chunk = section->data + (next - section->addr);

		done = next - section->addr;
		rem = section->len - (next - section->addr);

		graph( section->name, done, section->len );

		if( rem < CHUNK_SIZE ) {
			/* Pad out to 16 bytes long */
			uint8_t b[CHUNK_SIZE];
			uint8_t i;

			g_memmove( b, chunk, rem );
			for( i=rem; i<CHUNK_SIZE; i++ )
				b[i] = 0xaa;

			msp430_send_block( i2c_fd, 
					   0, 
					   next, 
					   b );
		}
		else
			msp430_send_block( i2c_fd, 
					   0, 
					   next, 
					   chunk );

		next = msp430_get_next_address( i2c_fd );

		/* May have failed */
		if( check_first && next < section->addr )
			next = section->addr;
	}

	graph( section->name, section->len, section->len );
	printf ("\n");
}

void msp430_confirm_crc( int i2c_fd )
{
	uint8_t buf[4];

	/* Format:
	 * 0-3: Password (currently ignored) */

	buf[0] = buf[1] = buf[2] = buf[3] = 0;

	while( sr_i2c_block_write( i2c_fd, commands[CMD_FW_CONFIRM], 4, buf, *msp430_fw_i2c_address ) < 0 );
}

static void graph( char *str, uint16_t done, uint16_t total )
{
	uint8_t w = 61 - strlen(str);
	float r = ((float)done)/((float)total);
	float p =  r * ((float)w);
	uint8_t i;

	printf("\r%s %4.4hx/%4.4hx (%3.0f%%) ", str, done, total,r*100.0);
	for( i=0; i < p; i++ ) {
		if( i == ((uint8_t)p) )
			putchar('>');
		else
			putchar('=');
	}

	for( ; i < w; i++ )
		putchar(' ');

	putchar('|');

	fflush(stdout);
}
