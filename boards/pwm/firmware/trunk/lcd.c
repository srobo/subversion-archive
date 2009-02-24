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
	lcd_delay_long(15);


/* RS  R/W DB7 DB6 DB5 DB4  Can't check BF before this instruction */
/*  0   0   0   0   1   1   Function set (8-bit interface) */
	lcd_cmd4(0x03);

/*     [Wait more than] */
/*     [     4.1ms    ] */
	lcd_delay_long(5);

/* RS  R/W DB7 DB6 DB5 DB4  Can't check BF before this instruction */
/*  0   0   0   0   1   1   Function set (8-bit interface) */
	lcd_cmd4(0x03);

/*     [Wait more than] */
/*     [     100us    ] */
	lcd_delay_long(1);

/* RS  R/W DB7 DB6 DB5 DB4  Can't check BF before this instruction */
/*  0   0   0   0   1   1   Function set (8-bit interface) */
	lcd_cmd4(0x03);

/*                          BF can be checked after the following */
/*                          instructions. When BF is not checked, */
/*                          the waiting time between instructions */
/*                          is longer than the execution time. */
/*                          (See Instruction set) */

/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   1   0   Function set (to 4-bit interface) */
	lcd_cmd4(0x02);

/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   1   0 */
	lcd_cmd4(0x02);
/*  0   0   N   F   *   *   Function set  [4-bit Interface      ] */
/*                                        [Specify display lines] */
	lcd_cmd4(0x04);



/* RS  R/W DB7 DB6 DB5 DB4                [and character font   ] */
/*  0   0   0   0   0   0                  These cannot be */
	lcd_cmd4(0x00);
/*  0   0   1   0   0   0   Display OFF    changed afterwards */
	lcd_cmd4(0x04); 


                             
/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   0   0    */
	lcd_cmd4(0x00);
/*  0   0   0   0   0   1   Display ON */
	lcd_cmd4(0x01);


/* RS  R/W DB7 DB6 DB5 DB4  */
/*  0   0   0   0   0   0    */
	lcd_cmd4(0x00);
/*  0   0   0   1  I/D  S   entry mode set */
	lcd_cmd4(0x03);


	for (i=0;i<buffer_len;i++)
		lcd_screens[0][i]=0;
	lcd_screens[0][0]='S';
	lcd_screens[0][0]='p';
	lcd_screens[0][0]='a';
	lcd_screens[0][0]='m';

}

void lcd_delay(void)
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

void lcd_delay_long(uint16_t time)
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
	for (i=0;i<buffer_len;i++)
	{
		lcd_screens[buffer_loc][i]= data[i];
	}
	if (current_screen == buffer_loc) /* re written to displayed bufferso redraw at next oppertunity */
		redraw =1;
}

void lcd_cmd4(uint8_t data)
{
	command_delay;
	rs_lo;
	P1OUT = (P1OUT & ~datapins )| (data & datapins);
	lcd_delay();		/* setup time */
	e_hi;
	lcd_delay();		/* data valid */
	e_lo;
}

void lcd_dat4(uint8_t data)
{
	command_delay;
	rs_hi;
	P1OUT = (P1OUT & ~datapins )| (data & datapins);
	lcd_delay();		/* setup time */
	e_hi;
	lcd_delay();		/* data valid */
	e_lo;
}


void lcd_address(uint8_t addr)
{
	lcd_cmd4( ( (addr&0x70)|0x80 )>>4 );
	lcd_cmd4(addr&0x0f);
	
}

void lcd_char(uint8_t data)
{
	lcd_dat4( ( (data&0x70)|0x80 )>>4 );
	lcd_dat4(data&0x0f);
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
      
		for (i=0;i<buffer_len;i++)
		{
			if (i == buffer_len/2)
				lcd_address(40); /* jump to second line */
			lcd_char(lcd_screens[requested_screen][i]);
		}
		current_screen = requested_screen;
	}
}
