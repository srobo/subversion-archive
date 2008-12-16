ifeq ($(origin CC),default)
CC = armeb-linux-gcc
endif

CFLAGS := -Wall

motor-test: motor-test.c

.PHONY: clean

clean:
	-rm -f motor-test
