#include "lcd.h"

/* display buffers */
uint8_t lcd_screens[buf_qty][buffer_len] 


void lcd_init( void )
{
	/* init pins */
	/* clear to a known state */
	P1OUT &= ~lcd_mask;
	/* enable outputs */
	P1DIR |= lcd_mask;


	/* init display */


/* 	    [Power ON] */

/* [  Wait more than 15ms  ] */
/* [after Vdd rises to 4.5v] */

/* RS  R/W DB7 DB6 DB5 DB4  Can't check BF before this instruction */
/*  0   0   0   0   1   1   Function set (8-bit interface) */

/*     [Wait more than] */
/*     [     4.1ms    ] */

/* RS  R/W DB7 DB6 DB5 DB4  Can't check BF before this instruction */
/*  0   0   0   0   1   1   Function set (8-bit interface) */

/*     [Wait more than] */
/*     [     100us    ] */

/* RS  R/W DB7 DB6 DB5 DB4  Can't check BF before this instruction */
/*  0   0   0   0   1   1   Function set (8-bit interface) */

/*                          BF can be checked after the following */
/*                          instructions. When BF is not checked, */
/*                          the waiting time between instructions */
/*                          is longer than the execution time. */
/*                          (See Instruction set) */

/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   1   0   Function set (to 4-bit interface) */

/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   1   0 */
/*  0   0   N   F   *   *   Function set  [4-bit Interface      ] */
/*                                        [Specify display lines] */
/* RS  R/W DB7 DB6 DB5 DB4                [and character font   ] */
/*  0   0   0   0   0   0                  These cannot be */
/*  0   0   1   0   0   0   Display OFF    changed afterwards */
                                        
/* RS  R/W DB7 DB6 DB5 DB4 */
/*  0   0   0   0   0   0    */
/*  0   0   0   0   0   1   Display ON */

/* RS  R/W DB7 DB6 DB5 DB4  */
/*  0   0   0   0   0   0    */
/*  0   0   0   1  I/D  S   entry mode set */


}


/* buffer is location, 0 indexed. data is pointer to string  */
void lcd_set_buffer(uint8_t buffer_loc, uint8_t* data)
{
	uint8_t i =0;
	for (i=0;i<buffer_len;i++)
	{
		lcd_screens[buffer_loc][i]= data[i];
	}
	lcd_redraw();
	
}

void lcd_cmd4(uint8_t data)
{
	P1OUT &= ~RS;
	(P1OUT & ~datapins) |= (data &= datapins);
	e hi
	delay
	e low
}

void lcd_dat4(uint8_t data)
{
		P1OUT |= RS;
}


void lcd_redraw(void)
{
}
