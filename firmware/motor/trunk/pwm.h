#ifndef __PWM_H
#define __PWM_H
#include <stdint.h>

#define PWM_MAX 328


/* The type for holding a PWM ratio */
typedef uint16_t pwm_ratio_t;

/* Initialise the pwm system */
void pwm_init();

/* Set the ratio */
void pwm_set( uint8_t channel, pwm_ratio_t r );

/* Get the ratio */
pwm_ratio_t pwm_get( uint8_t channel );

#endif	/* __PWM_H */
