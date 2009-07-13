default: check

.PHONY: check stock bom clean

bom: bom.htm

bom.htm: pwm-board.sch
	sr create_bom pwm-board.sch bom.htm

check:
	sr pcb_lint pwm-board.sch

stock:
	sr stockcheck pwm-board.sch

gerbers: gerbers/.built

gerbers/.built: pwm-board.brd
	sr export-gerber gerbers.conf pwm-board.brd
	touch -m gerbers/.built

clean:
	-rm -rf gerbers
	-rm -f bom.htm
