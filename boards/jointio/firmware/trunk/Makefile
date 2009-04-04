ARCH=msp430x2254
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH} -Wall
LDFLAGS :=

# for bool
CFLAGS += -I ./types

# flash430
LDFLAGS += -L ./flash430 -lflash430

H_FILES = common.h i2c.h init.h smbus_pec.h timer-b.h adc.h leds.h
C_FILES = main.c i2c.c init.c smbus_pec.c timer-b.c adc.c leds.c

world: jointio-top jointio-bottom

./flash430/libflash430.a:
	$(MAKE) -C ./flash430 ARCH="$(ARCH)" CC="$(CC)" CFLAGS="$(CFLAGS)"

jointio-top:  ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-top.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-top.x

jointio-bottom:  ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-bottom.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-bottom.x

.PHONY: clean 

clean:
	-rm -f jointio-{top,bottom}
	$(MAKE) -C ./flash430 clean