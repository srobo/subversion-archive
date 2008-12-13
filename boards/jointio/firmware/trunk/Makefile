ARCH=msp430x2254
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH} -Wall -O3
LDFLAGS :=

H_FILES = common.h i2c.h init.h smbus_pec.h timer-b.h adc.h flash.h
C_FILES = main.c i2c.c init.c smbus_pec.c timer-b.c adc.c flash.c

world: jointio-top jointio-bottom

jointio-top:  ${H_FILES} ${C_FILES} ${ARCH}-top.x
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,${ARCH}-top.x

jointio-bottom:  ${H_FILES} ${C_FILES} ${ARCH}-bottom.x
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,${ARCH}-bottom.x

.PHONY: clean

clean:
	-rm -f jointio-{top,bottom}
