ifeq ($(origin CC),default)
CC = armeb-linux-gcc
endif

CFLAGS := -Wall

jointio-test: jointio-test.c smbus_pec.c smbus_pec.h

.PHONY: clean

clean:
	-rm -f jointio-test
