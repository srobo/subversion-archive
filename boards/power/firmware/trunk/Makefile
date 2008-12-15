CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x169  -Wall -O3
LDFLAGS :=

H_FILES = hwinit.h led.h switch.h power.h i2c.h i2c_reg.h smbus_pec.h timed.h battery.h timer-a.h usart.h adc.h

C_FILES = main.c hwinit.c led.c switch.c power.c i2c.c i2c_reg.c smbus_pec.c timed.c battery.c timer-a.c usart.c adc.c

mod:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}

.PHONY: clean

clean:
	-rm -f mod
