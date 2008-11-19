#ifndef __LED_H
#define __LED_H

#include <stdint.h>
void led_init(void);
void led_set(uint8_t val);
uint8_t led_get(void);

/* #define goff P5OUT |= 0x01 */
/* #define gon P5OUT &= ~0x1 */
/* #define roff P5OUT |= 0x04 */
/* #define ron P5OUT &= ~0x4 */
/* #define rtog P5OUT ^= 0x4 */
/* #define gtog P5OUT ^= 0x1 */

/* Led's are: */
/*          pin */
/*          12       P1.0/TACLK I/O      LED0 */
/*          13       P1.1/TA0   I/O      LED1 */
/*          14       P1.2/TA1   I/O      LED2 */
/*          15       P1.3/TA2   I/O      LED3 */
#endif