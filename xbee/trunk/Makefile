CFLAGS := -g -Wall
LDFLAGS :=

# glib
CFLAGS += `pkg-config --cflags gtk+-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0`

main: main.c xbee.c xbee_at.c xbee.h xbee_at.h

.PHONY: clean

clean:
	-rm -f main
