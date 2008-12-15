#ifndef __POWER_H
#define __POWER_H

#include <stdint.h>
void pwr_init(void);
void slug_boot(uint8_t reboot);

void pwr_set_slug(uint8_t val);
uint8_t pwr_get_slug(void);

void pwr_set_servo(uint8_t val);
uint8_t pwr_get_servo(void);

void pwr_set_motor(uint8_t val);
uint8_t pwr_get_motor(void);

void delay(int16_t time);

/*          20       P2.0/ACLK  I/O      SLUG-PWR-EN */
/*          21       P2.1/TAINCLK I/O      SERVO-PWR-EN */
/*          22       P2.2/CAOUT/TA0 I/O      MOT-PWR-EN */
/*          51       P5.7/TBOUTH/SVSOUT I/O      SLUG-BOOT */



				/* the old pic code for ref */

   //rd0-3 switch
    //rd4-7 leds
    //re0 - big power motor fet thing (relay)
    //e1 - fet control servo rail
   //re2 - fet slug rail



/*     PORTC=0x01; // MUST BE set BEFORE UNTRISTATING ELSE SLUG BOOT!!! */
/*     TRISA=0XFF; */
/*     TRISB=0XFF; */
/*     TRISC=0XFE;// make slug pin Out */
/*     TRISD=0x0F; */
/*     TRISE = 0; */
/*     PORTE = 0; */
/*     delay(25); // wait for power to settle intpo regs before releasing shutdownpin (hopefully cure mode B slug poewr up fault) */
/*     PORTD =0; */
/*     PORTE = 0b110; // turn all power rails on */
/*     //IN real life will be 111 but changted to accomodate prototype2 relay error */

/*     //PORTE &= ~1; */
/*     delay(25); */
/*     PORTE = 0b111;  */
/*     delay(25); */
/*     PORTCbits.RC0=0;// blip slug */
/*     delay(5); */
/*     PORTCbits.RC0=1; // never press the button, ever!! (dont hold down) */
/*     delay(5); // JUST TO BE SURE NO POWER RAIL FLUCTUATION */

#endif
