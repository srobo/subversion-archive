NAME=xbee
#SMD = ../pcbfoot/smd-ic.py
#ODSTOOL= ./ods2trag.py
all: footprint symbol

footprint:
	./xbee.py $(NAME).fp

symbol:
	$(ODSTOOL) $(NAME).ods $(NAME).src
	for i in $$(ls *.src); do  tragesym $$i $${i%.src}.sym ; done

clean:
	-rm -f $(NAME)*.sym
	-rm -f $(NAME)*.src
	-rm -f $(NAME).fp