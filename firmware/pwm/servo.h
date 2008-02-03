#ifndef __SERVO_H
#define __SERVO_H
#include <stdint.h>

#define MIN_PULSE (uint16_t)(0.8*TICKS_PER_MS)
#define MIDDLE_PULSE (uint16_t)(1.5*TICKS_PER_MS)
#define MAX_PULSE (uint16_t)(2.3*TICKS_PER_MS)
#define PERIOD 60000

#define SERVO_NUMBER 6
#define TICKS_PER_MS (uint16_t)(PERIOD/20)

/* The servo pulse widths */
extern uint16_t servo_pulse[SERVO_NUMBER];

void servo_init( void );

/* Get the current PWM value for a servo */
unsigned int getServoPWM(unsigned char servo);

/* Set the PWM value for a servo.
 * Arguments:
 *  -       servo: The servo number.
 *  - pulse_width: Must be in the range 2260 to MAX_PULSE (0.8ms to 2.2ms). */
void setServoPWM(unsigned char servo, unsigned int pulse_width);

#endif /* __SERVO_H */
