#Path to checkout of tools
SMD-IC=/home/tom/project/gedashield/pcbfoot/smd-ic.py
DIL-IC=/home/tom/project/gedashield/pcbfoot/dil-ic.py
ODS2TRAG=/home/tom/project/gedashield/ods2trag/ods2trag.py

all: msp xbee camcon

xbee:
	SMD=$(SMD-IC) DIL=$(DIL-IC) ODSTOOL=$(ODS2TRAG) $(MAKE) -s -C xbee/ 

msp:
	SMD=$(SMD-IC) DIL=$(DIL-IC) ODSTOOL=$(ODS2TRAG) $(MAKE) -s -C msp430f16x/ 

camcon:
	SMD=$(SMD-IC) DIL=$(DIL-IC) ODSTOOL=$(ODS2TRAG) $(MAKE) -s -C camcon/ 


.PHONY: clean xbee msp camcon

clean::
	$(MAKE) clean -C xbee/
	$(MAKE) clean -C msp430f16x/
	$(MAKE) clean -C camcon/	
	
