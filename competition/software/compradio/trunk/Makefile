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

# mysql C library:
CFLAGS += -I/usr/include
LDFLAGS += -L/usr/lib/mysql -lmysqlclient

LDFLAGS += -Wl,--export-dynamic

C_FILES := compradio.c comp-mysql.c
H_FILES := comp-mysql.h comp-types.h

compradio: $(C_FILES) $(H_FILES) compradio.glade
	$(CC) $(C_FILES) -o compradio $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	-rm -f compradio
