CFLAGS := -Wall -O3
LDFLAGS := 

CFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --cflags libxb`
LDFLAGS += `pkg-config $(PKG_CONFIG_ARGS) --libs libxb`

xbout: xbout.c

.PHONY: clean

clean:
	-rm -f xbout
