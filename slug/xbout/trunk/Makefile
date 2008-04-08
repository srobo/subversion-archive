CFLAGS := -Wall -O3
LDFLAGS := 

CFLAGS += `pkg-config --cflags libxb`
LDFLAGS += `pkg-config --libs libxb`

xbout: xbout.c

.PHONY: clean

clean:
	-rm -f xbout
