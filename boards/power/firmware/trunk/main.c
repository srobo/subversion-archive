#include"device.h"
#include<io.h>
#include"hwinit.h"
#include<signal.h>
#include"led.h"
#include <stdint.h>
#include"switch.h"
#include"power.h"

int dummy =0; 			/* dummy variable to fix gdb bug */


int main(void)
{

  init_cpu();
  init_led();
  init_switch();
  pwr_init();

  while(1)
    {

 
/*       pwr_set_slug(get_switch() & 0x1 ); */
/*       pwr_set_servo(get_switch() & 0x2); */
/*       pwr_set_motor(get_switch() & 0x4 ); */
/*       set_led(pwr_get_slug() | (pwr_get_servo() <<1) | (pwr_get_motor() <<2)); */
    }
}

