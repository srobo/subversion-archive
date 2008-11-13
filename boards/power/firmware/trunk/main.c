#include"device.h"
#include<io.h>
#include"hwinit.h"
#include<signal.h>
#include"led.h"
#include <stdint>

int dummy =0; 			/* dummy variable to fix gdb bug */


int main(void)
{

  init_cpu();
  init_led();
  while(1)
    {
      set_led(0xFF);
      set_led(0x0);
    }

}

