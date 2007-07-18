# glib
CFLAGS += `pkg-config --cflags gtk+-2.0 gobject-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0 gobject-2.0`

test: test.c libxbee.a

libxbee.a: xbee-conn.o
	$(AR) r $@ $^




