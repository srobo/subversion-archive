#ifndef __TIMED_H
#define __TIMED_H

#include <stdint.h>

#define BOOT_TIME 600		/* 1 min */
#define SAFE_TIMEOUT 30		/* 3 sec */

extern uint8_t override;
extern uint8_t alive;

void stayingalive(void);
void alive_service(void);
void make_safe(void);
void safe_service(void);
void timer_override(void);

#endif
