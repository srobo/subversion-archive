#include "led.h"
#include"device.h"

void led_init(void)
{
  P1DIR |= 0x0f;
  P1OUT &= ~0x0f;
}
void led_set(uint8_t val)
{
	P1OUT = (P1OUT & 0xf0) | (val & 0x0f);
}

uint8_t led_get(void)
{
  return (P1IN & 0x0f);
}
