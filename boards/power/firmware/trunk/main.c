#include"device.h"
#include<io.h>
#include"hwinit.h"
#include<signal.h>
#include"led.h"
#include <stdint.h>
#include"switch.h"

int dummy =0; 			/* dummy variable to fix gdb bug */


int main(void)
{

  init_cpu();
  init_led();
  init_switch();
  while(1)
    {
      set_led(get_switch());
    }

}

