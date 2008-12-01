#include "switch.h"
#include"device.h"

void switch_init(void)
{
  P1DIR &= 0x0f;
  
}

uint8_t switch_get(void)
{
  return (P1IN &= 0xf0)>>4;
}
