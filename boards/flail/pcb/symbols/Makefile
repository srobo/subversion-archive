#Path to checkout of tools
SMD-IC=/home/chris/srobo/mainline/pcbfoot/smd-ic.py
DIL-IC=/home/chris/srobo/mainline/pcbfoot/dil-ic.py
ODS2TRAG=/home/chris/srobo/mainline/ods2trag/ods2trag.py

all: MAX1627

MAX1627:
	SMD=$(SMD-IC) DIL=$(DIL-IC) ODSTOOL=$(ODS2TRAG) $(MAKE) -s -C MAX1627/ 

.PHONY: clean MAX1627

clean::
	$(MAKE) clean -C MAX1627/
	
