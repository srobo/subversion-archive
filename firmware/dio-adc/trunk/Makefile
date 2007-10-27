CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x2234  -Wall
LDFLAGS :=

H_FILES = common.h i2c.h init.h dio.h adc.h
C_FILES = main.c i2c.c init.c dio.c adc.c

motor:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}


.PHONY: clean

clean:
	-rm -f motor
