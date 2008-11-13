#include "led.h"
#include"device.h"

void init_led(void)
{
  P1DIR |= 0x0f;
  
}
void set_led(uint8_t val)
{
  P1OUT = (val &= 0x0f);
}

uint8_t get_led(void)
{
  return (P1IN &= 0x0f);
}
