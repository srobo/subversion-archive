c2pymodule.so: c2pymodule.o smbus.o
	$(CC) -shared c2pymodule.o smbus.o $(CFLAGS) $(LDFLAGS) -o $@

c2pymodule.o: c2pymodule.c smbus.o
	$(CC) -o $@ -fPIC -c $< $(CFLAGS)

smbus.o: smbus_pec.c smbus_pec.h
	$(CC) -o $@ -fPIC -c $< $(CFLAGS)


.PHONY: clean

clean:
	rm -f c2pymodule.so
	rm -f c2pymodule.o
	rm -f smbus.o
