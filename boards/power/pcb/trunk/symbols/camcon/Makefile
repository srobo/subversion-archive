NAME=camcon
#SMD = ../pcbfoot/smd-ic.py
#ODSTOOL= ./ods2trag.py
all: symbol
symbol:
	$(ODSTOOL) $(NAME).ods $(NAME).src
	for i in $$(ls *.src); do  tragesym $$i $${i%.src}.sym ; done

clean:
	-rm -f $(NAME)*.sym
	-rm -f $(NAME)*.src
