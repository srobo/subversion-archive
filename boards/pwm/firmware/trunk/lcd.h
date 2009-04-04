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

#ifndef __LCD_H
#define __LCD_H
#include <stdint.h>
#include "hardware.h"

#define command_delay() do { lcd_delay_long(5); } while(0) /* should be 120us - write another delay later if problem*/

/* this is the length of the inner loop to get ms ticks */
/* 	clock speed = 12MHz - 83ns */
/* 	so appx 12048 clocks for 1ms */
/*      but loop is more likely about 5 ticks */
/*      so try 2400 */
#define MS_SCALE 12048	


#define buf_qty 4
#define lcd_buffer_len 32

#define E 0x04
#define RS 0x08
#define datapins 0xf0
#define lcd_mask 0xfc
/* all pins are PORT 1  */

#define e_lo P1OUT |= E
#define e_hi P1OUT &= ~E

#define rs_lo P1OUT |= RS
#define rs_hi P1OUT &= ~RS

uint8_t current_screen;
uint8_t requested_screen;
uint8_t redraw;
/* display buffers */
uint8_t lcd_screens[buf_qty][lcd_buffer_len]; 

void lcd_init( void );
void lcd_set_buffer(uint8_t buffer_loc, uint8_t* data);
void lcd_service(void);
uint8_t lcd_csum(uint8_t pos);

#endif /* __LCD_H */
