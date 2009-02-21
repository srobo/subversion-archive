#ifndef __LCD_H
#define __LCD_H
#include <stdint.h>


void lcd_init( void );

#define buf_qty 4
#define buffer_len 16

#define E 0x02
#define RS 0x04
#define datapins 0xf0
#define lcd_mask 0xfc



/*                              RS  R/W DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0 */
/*                              ==  === === === === === === === === === */
/* Clear Display                 0   0   0   0   0   0   0   0   0   1 */

/* Return Home                   0   0   0   0   0   0   0   0   1   * */

/* Entry Mode Set                0   0   0   0   0   0   0   1  I/D  S */

/* Display ON/OFF                0   0   0   0   0   0   1   D   C   B */

/* Cursor and Display Shift      0   0   0   0   0   1  S/C R/L  *   * */

/* Function Set                  0   0   0   0   1   DL  N   F   *   * */

/* Set CG RAM address            0   0   0   1   A   A   A   A   A   A */

/* Set DD RAM address            0   0   1   A   A   A   A   A   A   A */

/* Read busy flag and address    0   1   BF  A   A   A   A   A   A   A */

/* Write data to CG or DD RAM    1   0   D   D   D   D   D   D   D   D */

/* Read data from CG or DD RAM   1   1   D   D   D   D   D   D   D   D */



void lcd_init( void );
void lcd_set_buffer(uint8_t buffer_loc, uint8_t* data);
void lcd_redraw(void)

#endif /* __LCD_H */
