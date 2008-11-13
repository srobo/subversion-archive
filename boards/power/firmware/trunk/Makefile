CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x169  -Wall -O3
LDFLAGS :=

H_FILES = hwinit.h led.h switch.h power.h
C_FILES = main.c hwinit.c led.c switch.c power.c

mod:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}

.PHONY: clean

clean:
	-rm -f mod
