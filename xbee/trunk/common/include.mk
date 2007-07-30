CLEAN += $(addprefix common/,xb-fd-source.o common-fd.o)

common/xb-fd-source.o: $(addprefix common/,xb-fd-source.c xb-fd-source.h)

common/common-fd.o: $(addprefix common/,common-fd.c common-fd.h)
