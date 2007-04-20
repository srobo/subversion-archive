CFLAGS := -g -Wall
LDFLAGS := 

# glib
CFLAGS += `pkg-config --cflags gtk+-2.0 gobject-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0 gobject-2.0`

# CFLAGS += -fmudflap
# LDFLAGS += -lmudflap

C_FILES := xbee.c xbee_at.c xbee-incoming.c xbee-server.c
H_FILES := xbee.h xbee_at.h xbee-incoming.h xbee-server.h

main: $(C_FILES) $(H_FILES)

.PHONY: clean

clean:
	-rm -f main
