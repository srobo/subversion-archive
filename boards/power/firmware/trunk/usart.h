#ifndef __LED_H
#define __LED_H
#include <stdint.h>
void usart_init(void);
static inline void putc(char c);
static inline unsigned int getc(void);

#endif
