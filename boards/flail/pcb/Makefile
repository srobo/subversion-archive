#board name
BOARD=flail

all: bom pcb

bom: 
	gnetlist -g bom -o $(BOARD).bom $(BOARD).sch	
	-rm -f gnetlist.log

pcb: 	 
	gsch2pcb -f project

.PHONY: clean bom pcb

clean: 
	-rm -f $(BOARD).bom	
	-rm -f $(BOARD).{net,cmd,new.pcb}
	-rm -f gnetlist.log
