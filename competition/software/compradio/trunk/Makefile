CFLAGS := -Wall -O3
LDFLAGS :=

CFLAGS += `pkg-config --cflags libxb`
LDFLAGS += `pkg-config --libs libxb`

compradio: compradio.c teams.o

teams.o: teams.c teams.h

.PHONY: clean

clean:
	-rm -f compradio teams.o
