default: check

.PHONY: check stock bom clean

bom: bom.htm

bom.htm: jointio.sch
	sr create_bom jointio.sch bom.htm

check:
	sr pcb_lint jointio.sch

stock:
	sr stockcheck jointio.sch

gerbers: gerbers/.built

gerbers/.built: jointio.brd
	sr export-gerber gerbers.conf jointio.brd
	touch -m gerbers/.built

clean:
	-rm -rf gerbers
	-rm -f bom.htm
