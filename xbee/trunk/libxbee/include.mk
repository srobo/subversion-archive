CLEAN += $(addprefix libxbee/,libxbee.a xbee-conn.o test)

libxbee/test: libxbee/test.c libxbee/libxbee.a

libxbee/libxbee.a: libxbee/xbee-conn.o common/xb-fd-source.o
#libxbee/libxbee.a: libxbee/xbee-conn.o
	$(AR) r $@ $^




