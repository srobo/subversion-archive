default: check

.PHONY: check stock bom

bom: bom.htm

bom.htm: motor.sch
	sr create_bom motor.sch bom.htm

check:
	sr pcb_lint motor.sch

stock:
	sr stockcheck motor.sch

gerbers: gerbers/.built

gerbers/.built: motor.brd
	sr export-gerber gerbers.conf motor.brd
	touch -m gerbers/.built
