#Path to checkout of tools
SMD-IC=/home/tom/project/gedashield/pcbfoot/smd-ic.py
DIL-IC=/home/tom/project/gedashield/pcbfoot/dil-ic.py
ODS2TRAG=/home/tom/project/gedashield/ods2trag/ods2trag.py

# grep ^P xc3s50-5tqg144c*.sym | wc

all: msp xbee

xbee:
	SMD=$(SMD-IC) DIL=$(DIL-IC) ODSTOOL=$(ODS2TRAG) $(MAKE) -s -C xbee/ 

msp:
	SMD=$(SMD-IC) DIL=$(DIL-IC) ODSTOOL=$(ODS2TRAG) $(MAKE) -s -C msp430f16x/ 

.PHONY: clean xbee

clean::
	$(MAKE) clean -C xbee/
	
