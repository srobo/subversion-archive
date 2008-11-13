#include "switch.h"
#include"device.h"

void init_switch(void)
{
  P1DIR &= 0x0f;
  
}

uint8_t get_switch(void)
{
  return (P1IN &= 0xf0)<<4;
}
