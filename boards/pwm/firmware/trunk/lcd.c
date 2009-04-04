/*   Copyright (C) 2009 Tom Bennellick

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

#include "lcd.h"
#include <stdint.h>

#if USE_LCD
static void lcd_delay(void);
static void lcd_delay_long(uint16_t time);
static void lcd_cmd4(uint8_t data);
static void lcd_dat4(uint8_t data);
static void lcd_address(uint8_t addr);
static void lcd_char(uint8_t data);


void lcd_init( void )
{
	uint8_t i;
	/* init pins */
	/* clear to a known state */
	P1OUT &= ~lcd_mask;
	/* enable outputs */
	P1DIR |= lcd_mask;


	/* init display */


/* 	    [Power ON] */

/* [  Wait more than 15ms  ] */
/* [after Vdd rises to 4.5v] */
	lcd_delay_long(150);


/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   1   0   Function set (to 4-bit interface) */
	lcd_cmd4(0x02);

/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   1   0 */
	lcd_cmd4(0x02);

/*  0	0   0   0   N   F   *   *   Function set  [4-bit Interface      ] */
/*                                        [Specify display lines] */
	lcd_cmd4(0x8);

	lcd_delay();

/* RS  R/W DB7 DB6 DB5 DB4  	Display OFF             */
/*  0   0   0   0   0   0       */
	lcd_cmd4(0x00);
/*  0   0   1   1   1   1   	Cursor, Blink, On */
	lcd_cmd4(0x0f); 

	lcd_delay();

/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   0   0    	Clear Display*/
	lcd_cmd4(0x00);
/*  0   0   0   0   0   1  	Clear Display cont.*/
	lcd_cmd4(0x01);
	lcd_delay_long(100);	

/* RS  R/W DB7 DB6 DB5 DB4  */
/*  0   0   0   0   0   0    */
	lcd_cmd4(0x00);
/*  0   0   0   1  I/D  S   entry mode set */
	lcd_cmd4(0x03);

	lcd_delay_long(100);

	lcd_address(0x0);

	for (i=0;i<lcd_buffer_len;i++)
		lcd_screens[0][i]=' ';
	lcd_screens[0][0]='S';
	lcd_screens[0][1]='p';
	lcd_screens[0][2]='a';
	lcd_screens[0][3]='m';
	
	redraw = 1;	
}
static void lcd_delay(void)
{
	nop();
	nop();
	nop();
	nop();
	nop();
	nop();			/* only 7 are nessesary but 10 just to be safe */
	nop();
	nop();
	nop();
	nop();
	
}

static void lcd_delay_long(uint16_t time)
{
	uint16_t in_tick;
	uint16_t out_tick;
	for (out_tick=0;out_tick<time;out_tick++)
	{
		for(in_tick=0; in_tick < MS_SCALE ; in_tick++)
			nop();
	}
}

/* use this function to update a screen, it copys data which will be displayed later by the main service entry */
/* buffer is location, 0 indexed. data is pointer to string  */
void lcd_set_buffer(uint8_t buffer_loc, uint8_t* data)
{
	uint8_t i =0;
	for (i=0;i<lcd_buffer_len;i++)
	{
		lcd_screens[buffer_loc][i]= data[i];
	}
	if (current_screen == buffer_loc) /* re written to displayed bufferso redraw at next oppertunity */
		redraw =1;
}

static void lcd_cmd4(uint8_t data)
{
	rs_lo;
	P1OUT = (P1OUT & 0x0f )| (~data << 4);
	lcd_delay_long(1);		/* setup time */
	e_hi;
	lcd_delay_long(1);		/* data valid */
	e_lo;
	command_delay();
}

static void lcd_dat4(uint8_t data)
{
	rs_hi;
	P1OUT = (P1OUT & 0x0f )| (~data << 4);
	lcd_delay_long(1);		/* setup time */
	e_hi;
	lcd_delay_long(1);		/* data valid */
	e_lo;
	command_delay();
}


static void lcd_address(uint8_t addr)
{
	lcd_cmd4( ( (addr & 0x70)|0x80 )>>4 );
	lcd_cmd4(addr & 0x0f);
}

static void lcd_char(uint8_t data)
{
	lcd_dat4( data >> 4);
	lcd_dat4(data & 0x0f);
}

void lcd_service(void)
{
	uint8_t i =0;
	
	if ((current_screen == requested_screen) && (redraw == 0))
		/* everything is upto date */
		return;
	else
	{
		lcd_address(0);
      
		for (i=0;i<lcd_buffer_len;i++)
		{
			if (i == lcd_buffer_len/2)
				lcd_address(40); /* jump to second line */
			lcd_char(lcd_screens[requested_screen][i]);
		}
		current_screen = requested_screen;
		redraw = 0;
	}
}

uint8_t lcd_csum(uint8_t pos)
{	
	uint8_t i =0;
	uint8_t total=0;
	for (i=0;i<lcd_buffer_len;i++)
	{
		total += lcd_screens[pos][i];
	}
	return total;
}

#else
void lcd_init( void )
{
}

void lcd_set_buffer(uint8_t buffer_loc, uint8_t* data)
{
}

void lcd_service(void)
{
}

uint8_t lcd_csum(uint8_t pos)
{
	return 0;
}
#endif
