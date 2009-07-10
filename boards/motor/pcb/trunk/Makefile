motor_bom.htm: motor.sch
	sr create_bom motor.sch motor_bom.htm

.PHONY: check stock

check:
	sr pcb_lint motor.sch

stock:
	sr stockcheck motor.sch
