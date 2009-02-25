/* Routines for controlling the LEDs */
#ifndef __LED_H
#define __LED_H
#include <stdint.h>

void led_init(void);

/* Set the LEDs to the given value.
 * Arguments:
 *  - val: 4 least significant bits map to LEDs.
 *         '0' for off.  '1' for on. */
void led_set(uint8_t val);

/* Get the current LED state.
 * The 4 least significant bits of return value map to LEDs.
 * '0' for off.  '1' for on. */
uint8_t led_get(void);

/* #define goff P5OUT |= 0x01 */
/* #define gon P5OUT &= ~0x1 */

#define toga
#define togb
#define togc
#define togd

/* Led's are: */
/*          pin */
/*          12       P1.0/TACLK I/O      LED0 */
/*          13       P1.1/TA0   I/O      LED1 */
/*          14       P1.2/TA1   I/O      LED2 */
/*          15       P1.3/TA2   I/O      LED3 */

#endif	/* __LED_H */
