#ifndef __FLASH430_DEV_H
#define __FLASH430_DEV_H
#include <io.h>

/* In the future, this may use the device peripheral constants, like
 * __MSP430_HAS_FLASH__.  But for now, we're doing it based on the
 * device */

#ifdef __MSP430_2254__
#define FLASH_AREA_0 0xc000
#define FLASH_AREA_1 0xe000
#define FLASH_AREA_LEN 0x1e00
#endif

#ifdef __MSP430_2234__
#define FLASH_AREA_0 0xe000
#define FLASH_AREA_1 0xf000
#define FLASH_AREA_LEN 0xe00
#endif

#ifdef __MSP430_169__
#define FLASH_AREA_0 0x1200
#define FLASH_AREA_1 0x8900
#define FLASH_AREA_LEN 0x7500
#endif

#ifdef __MSP430_1611__
#define FLASH_AREA_0 0x4000
#define FLASH_AREA_1 0xa000
#define FLASH_AREA_LEN 0x5e00
#endif

#endif /* __FLASH430_DEV_H */
