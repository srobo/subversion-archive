ARCH=msp430x169
CC := msp430-gcc
CFLAGS := -g -mmcu=${ARCH} -Wall -O3
LDFLAGS :=

# for bool
CFLAGS += -I ./types

# flash430
LDFLAGS := -L ./flash430 -lflash430

H_FILES = hwinit.h led.h switch.h power.h i2c.h i2c_reg.h smbus_pec.h timed.h battery.h timer-a.h usart.h adc.h isense.h xbee.h

C_FILES = main.c hwinit.c led.c switch.c power.c i2c.c i2c_reg.c smbus_pec.c timed.c battery.c timer-a.c usart.c adc.c isense.c xbee.c

world: power-bottom power-top

./flash430/libflash430.a:
	$(MAKE) -C ./flash430 ARCH="$(ARCH)" CC="$(CC)" CFLAGS="$(CFLAGS)"

power-bottom:  ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-bottom.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-bottom.x

power-top:  ${H_FILES} ${C_FILES} ./flash430/lkr/${ARCH}-bottom.x ./flash430/libflash430.a
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,./flash430/lkr/${ARCH}-top.x

dumps: text-top text-bottom

text-top: power-top
	msp430-objcopy -j .text power-top -O binary text-top

text-bottom: power-bottom
	msp430-objcopy -j .text power-bottom -O binary text-bottom

.PHONY: clean

clean:
	-rm -f power-{bottom,top} test-{top,bottom}
	$(MAKE) -C flash430 clean