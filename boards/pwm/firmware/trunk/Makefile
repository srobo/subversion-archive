ARCH := msp430x2254
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH}  -Wall -O3
LDFLAGS :=

# for bool
CFLAGS += -I ./types

# flash430
LDFLAGS := -L ./flash430 -lflash430

include .config

ifeq ($(strip $(SWEEP_SERVOS)),y)
CFLAGS += -DSWEEP_SERVOS=1
else
CFLAGS += -DSWEEP_SERVOS=0
endif

ifeq ($(strip $(USE_CHECKSUMS)),y)
CFLAGS += -DUSE_CHECKSUMS=1
else
CFLAGS += -DUSE_CHECKSUMS=0
endif

ifeq ($(strip $(USE_LCD)),y)
CFLAGS += -DUSE_LCD=1
else
CFLAGS += -DUSE_LCD=0
endif


H_FILES = servo.h sweep.h hardware.h common.h i2c.h smbus_pec.h timer-b.h lcd.h \
	railmon.h watchdog.h led.h
C_FILES = main.c servo.c sweep.c i2c.c smbus_pec.c timer-b.c lcd.c \
	railmon.c

world: pwm-bottom pwm-top

./flash430/libflash430.a:
	$(MAKE) -C ./flash430 ARCH="$(ARCH)" CC="$(CC)" CFLAGS="$(CFLAGS)"

pwm-bottom: ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-bottom.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-bottom.x

pwm-top: ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-top.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-top.x

.PHONY: clean

clean:
	-rm -f pwm-{bottom,top}
	$(MAKE) -C ./flash430 clean
