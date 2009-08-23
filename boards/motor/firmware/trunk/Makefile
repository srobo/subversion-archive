ARCH=msp430x2254
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH} -Wall -O3

# for bool
CFLAGS += -I ./types

# flash430
LDFLAGS := -L ./flash430 -lflash430

H_FILES = common.h i2c.h motor.h pwm.h smbus_pec.h timer-b.h leds.h \
	h-bridge.h sensors/sensor.h controllers/controllers.h \
	controllers/unity.h sensors/null.h control.h sensors/ads5030.h \
	sensors/ads5030_state.h controllers/pid.h controllers/pid_state.h \
	i2c_desc.h
C_FILES = main.c i2c.c motor.c pwm.c smbus_pec.c timer-b.c leds.c \
	h-bridge.c controllers/unity.c sensors/null.c control.c \
	sensors/ads5030.c controllers/pid.c i2c_desc.c sensors/sensor.c \
	controllers/controllers.c

world: motor-bottom motor-top

./flash430/libflash430.a:
	$(MAKE) -C ./flash430 ARCH="$(ARCH)" CC="$(CC)" CFLAGS="$(CFLAGS)"

motor-bottom: ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-bottom.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-bottom.x

motor-top: ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-top.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-top.x

.PHONY: clean

clean:
	-rm -f motor-{bottom,top}
	$(MAKE) -C flash430 clean