CFLAGS := -Wall -O3
LDFLAGS :=

# CFLAGS += `pkg-config --cflags libxb`
# LDFLAGS += `pkg-config --libs libxb`

CFLAGS := `pkg-config --cflags libglade-2.0`
LDFLAGS := `pkg-config --libs libglade-2.0`

LDFLAGS += -Wl,--export-dynamic

compradio: compradio.c compradio.glade
	$(CC) compradio.c -o compradio $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	-rm -f compradio
