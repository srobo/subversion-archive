NAME=MAX1627
all: footprint symbol

footprint:
	sr smd-ic.py MAX1627 10 0 4 0 4 0.55 1.27 4 5 mm MAX1627.pcb 
symbol:
	sr ods2trag.py $(NAME).ods $(NAME).src
	for i in $$(ls *.src); do  tragesym $$i $${i%.src}.sym ; done

clean:
	-rm -f $(NAME)*.sym
	-rm -f $(NAME)*.src
	-rm -f $(NAME).fp
