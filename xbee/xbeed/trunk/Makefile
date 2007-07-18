CFLAGS := -g -Wall
LDFLAGS := 

# glib
CFLAGS += `pkg-config --cflags gtk+-2.0 gobject-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0 gobject-2.0`

# CFLAGS += -fmudflap
# LDFLAGS += -lmudflap

C_FILES := xbee-module.c xbee_at.c xbee-server.c xbee-client.c xb-fd-source.c
H_FILES := xbee-module.h xbee_at.h xbee-server.h xbee-client.h xb-fd-source.h

main: $(C_FILES) $(H_FILES)

.PHONY: clean

clean:
	-rm -f main
