default: check

.PHONY: check stock bom clean

bom: bom.htm

bom.htm: PWR1.sch
	sr create_bom PWR1.sch bom.htm

check:
	sr pcb_lint PWR1.sch

stock:
	sr stockcheck PWR1.sch

gerbers: gerbers/.built

gerbers/.built: PWR1.brd
	sr export-gerber gerbers.conf PWR1.brd
	touch -m gerbers/.built

clean:
	-rm -rf gerbers
	-rm -f bom.htm
