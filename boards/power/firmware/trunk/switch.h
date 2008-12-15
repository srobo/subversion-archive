/* Routines for accessing the DIP switches and push button */
#ifndef __SWITCH_H
#define __SWITCH_H
#include <stdint.h>

extern uint8_t button_pressed;

void switch_init(void);

/* Returns the DIP switch values.
   The least significant 4 bits of the returned value
   map to each switch. */
uint8_t switch_get(void);

uint8_t switch_get_button(void);

/* switch mapping */
/*          pin */
/*          16       P1.4/SMCLK I/O      SW0 */
/*          17       P1.5/TA0   I/O      SW1 */
/*          18       P1.6/TA1   I/O      SW2 */
/*          19       P1.7/TA2   I/O      SW3 */

#endif	/* __SWITCH_H */
