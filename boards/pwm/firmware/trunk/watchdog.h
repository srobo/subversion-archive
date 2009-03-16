#ifndef __WATCHDOG_H
#define __WATCHDOG_H

/* ACLK, 512 counts */
#define WATCHDOG_SETTINGS (WDTSSEL | WDTIS_2)

#if (USE_WATCHDOG)
#define watchdog_clear() do { WDTCTL = WATCHDOG_SETTINGS | WDTCNTCL | WDTPW; } while (0)
#else
#define watchdog_clear() do {} while(0)
#endif

#endif	/* __WATCHDOG_H */
