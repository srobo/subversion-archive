/*   Copyright (C) 2007 Robert Spanton and Chris Cross

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
#ifndef __ADC_H
#define __ADC_H
#include <stdint.h>

#define INPUT_THRESHOLD 1024/2

void adc_init ( void );

/* Perform the ADC samples */
void adc_sample(void);

/* Buffer of ADC readings */
extern uint16_t adc_buffer[8];

#endif 	/* __ADC_H  */
