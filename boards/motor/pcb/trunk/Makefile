default: check

motor_bom.htm: motor.sch
	sr create_bom motor.sch motor_bom.htm

.PHONY: check stock

check:
	sr pcb_lint motor.sch

stock:
	sr stockcheck motor.sch

gerbers: gerbers/.built

gerbers/.built: motor.brd
	sr export-gerber gerbers.conf motor.brd
	touch -m gerbers/.built
