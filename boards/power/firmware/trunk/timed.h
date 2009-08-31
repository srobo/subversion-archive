#ifndef __TIMED_H
#define __TIMED_H

#include <stdint.h>

#define BOOT_TIME 600		/* 1 min */
#define SAFE_TIMEOUT 30		/* 3 sec */

extern uint8_t override;

enum {
	STANDBY = 0,
	USER = 1,
	GAME = 2,
	END = 4
};

void timed_init(void);
void reset_cutoff(void);
void alive_service(void);
void game_enable(void);
void safe_service(void);
void user_enable(void);

#endif
