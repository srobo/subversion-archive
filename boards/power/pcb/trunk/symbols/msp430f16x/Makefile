NAME=msp430f16x
#SMD = ../pcbfoot/smd-ic.py
#ODSTOOL= ./ods2trag.py
all: footprint symbol

footprint:
	$(SMD) $(NAME) 11.25 11.25 16 16 0.8 0.3 0.5 10 10 mm $(NAME).fp
symbol:
	$(ODSTOOL) $(NAME).ods $(NAME).src
	for i in $$(ls *.src); do  tragesym $$i $${i%.src}.sym ; done

clean:
	-rm -f $(NAME)*.sym
	-rm -f $(NAME)*.src
	-rm -f $(NAME).fp