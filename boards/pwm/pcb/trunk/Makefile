default: check

pwm_bom.htm: pwm-board.sch
	sr create_bom pwm-board.sch pwm_bom.htm

.PHONY: check stock

check:
	sr pcb_lint pwm-board.sch

stock:
	sr stockcheck pwm-board.sch

gerbers: gerbers/.built

gerbers/.built: pwm-board.brd
	sr export-gerber gerbers.conf pwm-board.brd
	touch -m gerbers/.built
