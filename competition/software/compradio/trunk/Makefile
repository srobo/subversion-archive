CFLAGS := -Wall -O3
LDFLAGS :=

# CFLAGS += `pkg-config --cflags libxb`
# LDFLAGS += `pkg-config --libs libxb`
CFLAGS += `pkg-config --cflags gobject-2.0` 
CFLAGS += -I$(LIBXB_DIR)/libxbee -I$(LIBXB_DIR)/common
LDFLAGS += `pkg-config --libs gobject-2.0`
LDFLAGS += -L$(LIBXB_DIR)/libxbee -lxbee

CFLAGS += `pkg-config --cflags libglade-2.0`
LDFLAGS += `pkg-config --libs libglade-2.0`

LDFLAGS += -Wl,--export-dynamic

compradio: compradio.c compradio.glade
	$(CC) compradio.c -o compradio $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	-rm -f compradio
