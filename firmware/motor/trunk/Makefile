CFLAGS := -g -mmcu=msp430x2234
LDFLAGS :=

H_FILES = common.h i2c.h init.h motor.h pwm.h
C_FILES = main.c i2c.c init.c motor.c pwm.c

motor:  ${H_FILES} ${C_FILES}
	msp430-gcc -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}
 
