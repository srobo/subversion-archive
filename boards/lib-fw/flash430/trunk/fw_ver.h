#ifndef __FW_VER_H
#define __FW_VER_H
#include <stdint.h>

extern const uint16_t firmware_version;

#define FIRMWARE_REBOOT_MAGIC 0x85
/* If this value is equal to the above magic number,
   then the device has just been rebooted. */
extern uint8_t firmware_rebooted;

#endif	/* __FW_VER_H */
