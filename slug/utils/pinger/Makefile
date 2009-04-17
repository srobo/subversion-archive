CFLAGS ?= -Wall -O3
LDFLAGS ?= 

# CFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --cflags libxb`
# LDFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --libs libxb`
CFLAGS += -I$(LIBXB_DIR)/libxbee -I$(LIBXB_DIR)/common
LDFLAGS += -L$(LIBXB_DIR)/libxbee -lxbee

CFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --cflags glib-2.0 gobject-2.0`
LDFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --libs glib-2.0 gobject-2.0`


pinger: pinger.c

.PHONY: clean

clean:
	-rm -f pinger

