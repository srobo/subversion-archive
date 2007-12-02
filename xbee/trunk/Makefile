CFLAGS += -g -Wall
CLEAN :=

# common
CFLAGS += -I./common

# glib
CFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --cflags gobject-2.0`
LDFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --libs gobject-2.0`

all: xbd/xbd libxbee/test

include ./*/include.mk

.PHONY: clean

clean:
	rm -f $(CLEAN)