#ifndef __TIMED_H
#define __TIMED_H

#define BOOT_TIME 600		/* 1 min */
#define SAFE_TIMEOUT 30		/* 3 sec */
void stayingalive(void);
void alive_service(void);
void safe_service(void);

#endif
