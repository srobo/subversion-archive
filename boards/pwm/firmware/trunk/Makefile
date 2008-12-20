ARCH := msp430x2234
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH}  -Wall -O3
LDFLAGS :=

# for bool
CFLAGS += -I ./types

# flash430
LDFLAGS := -L ./flash430 -lflash430

H_FILES = servo.h sweep.h hardware.h common.h i2c.h smbus_pec.h timer-b.h
C_FILES = main.c servo.c sweep.c i2c.c smbus_pec.c timer-b.c

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
