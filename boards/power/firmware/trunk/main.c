#include"device.h"
#include<io.h>
#include"hwinit.h"
#include<signal.h>



int poo =0; 			/* dummy variable to fix gdb bug */


/* init routine flags */



int main(void)
{

  init_wd();
  init_io();
  init_osc();
  
  while(1)
    {
    }

}

