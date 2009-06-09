#board name
BOARD=flail

all: bom 

bom: 
	gnetlist -g bom -o $(BOARD).bom $(BOARD).sch	
	-rm -f gnetlist.log

pcb: 	 
	gsch2pcb -f project

.PHONY: clean bom pcb

clean: 
	-rm -f $(BOARD).bom	
