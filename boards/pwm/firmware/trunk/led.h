#ifndef __LED_H
#define __LED_H
#include "common.h"

#define LED_ACT_PIN 0x20

#define led_init() do { P2DIR |= LED_ACT_PIN; \
	P2OUT &= ~LED_ACT_PIN; } while (0)

/* Turn the activity LED on */
#define led_activity_on() do { P2OUT |= LED_ACT_PIN; } while (0)

/* Turn the activity LED off */
#define led_activity_off() do { P2OUT &= ~LED_ACT_PIN; } while (0)

#endif	/* __LED_H */
