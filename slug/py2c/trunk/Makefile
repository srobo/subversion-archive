PY_CFLAGS ?= -I/usr/include/python2.5

c2pymodule.so: c2pymodule.o
	$(CC) -shared $< $(LDBFLAGS) -o $@

c2pymodule.o: c2pymodule.c
	$(CC) -o $@ -fPIC -c $< $(PY_CFLAGS)

.PHONY: clean

clean:
	-rm -f c2pymodule.{so,o}
