ARCH=msp430x2234
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH} -Wall
LDFLAGS :=

H_FILES = common.h i2c.h motor.h pwm.h smbus_pec.h timer-b.h

C_FILES = main.c i2c.c motor.c pwm.c smbus_pec.c timer-b.c

world: motor-bottom motor-top

motor-bottom: ${H_FILES} ${C_FILES} ${ARCH}-bottom.x
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,${ARCH}-bottom.x

motor-top: ${H_FILES} ${C_FILES} ${ARCH}-top.x
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,${ARCH}-top.x

.PHONY: clean

clean:
	-rm -f motor-{bottom,top}
