CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x2234  -Wall -O3
LDFLAGS :=

H_FILES = common.h i2c.h init.h smbus_pec.h timer-b.h
C_FILES = main.c i2c.c init.c smbus_pec.c timer-b.c

motor:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}


.PHONY: clean

clean:
	-rm -f motor
