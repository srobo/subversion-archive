#ifndef __ADC_H
#define __ADC_H
#include "device.h"
#include "types.h"

extern uint16_t voltage;
extern uint16_t current;


/* Initialises the ADC peripheral */
void adc_init( void );


#endif	/* __ADC_H */
