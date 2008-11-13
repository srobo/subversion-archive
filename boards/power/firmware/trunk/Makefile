CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x169  -lm #-Wall -O3
LDFLAGS :=

H_FILES = 
C_FILES = main.c

mod:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}

.PHONY: clean

clean:
	-rm -f mod
