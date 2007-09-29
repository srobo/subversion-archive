CFLAGS := -g -Wall
LDFLAGS := 
CLEAN :=

# common
CFLAGS += -I./common

# glib
CFLAGS += `pkg-config --cflags gtk+-2.0 gobject-2.0`
LDFLAGS += `pkg-config --libs gtk+-2.0 gobject-2.0`

all: xbd/xbd libxbee/test compcode/compcode

include ./*/include.mk

.PHONY: clean

clean:
	rm -f $(CLEAN)