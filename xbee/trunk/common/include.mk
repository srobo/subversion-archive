CLEAN += $(addprefix common/,xb-fd-source.o)

common/xb-fd-source.o: $(addprefix common/,xb-fd-source.c xb-fd-source.h)
