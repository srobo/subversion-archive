#ifndef __FLASH430_DEV_H
#define __FLASH430_DEV_H

/* In the future, this may use the device peripheral constants, like
 * __MSP430_HAS_FLASH__.  But for now, we're doing it based on the
 * device */

#ifdef __MSP430_2254__
#include <msp430x22x4.h>
#endif

#ifdef __MSP430_2234__
#include <msp430x22x4.h>
#endif

#endif /* __FLASH430_DEV_H */
