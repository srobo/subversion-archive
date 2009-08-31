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

#endif
