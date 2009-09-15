default: check

.PHONY: check stock bom clean

bom: bom.htm

bom.htm: as5030.sch
	sr create_bom as5030.sch bom.htm

check:
	sr pcb_lint as5030.sch

stock:
	sr stockcheck as5030.sch

gerbers: gerbers/.built

gerbers/.built: as5030.brd
	sr export-gerber gerbers.conf as5030.brd
	touch -m gerbers/.built

clean:
	-rm -rf gerbers
	-rm -f bom.htm
