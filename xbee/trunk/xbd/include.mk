CLEAN += $(addprefix xbd/,xbd *.o)

xbd/xbd: common/xb-fd-source.o $(addprefix xbd/,xbee-module.c xbee_at.c xbee-server.c xbee-client.c) \
$(addprefix xbd/,xbee-module.h xbee_at.h xbee-server.h xbee-client.h)
