all: power motor pwm jointio

power_dir=../power/pcb/trunk/
motor_dir=../motor/pcb/trunk/
pwm_dir=../pwm/pcb/trunk/
jointio_dir=../jointio/pcb/trunk/
bom_dir=../shipping/2008/bom/in/

power: 
	mkdir -p ${power_dir}/gerbers
	./makegerb.sh 4 ${power_dir}PWR1.brd ${power_dir}gerbers/power
	./getprt.sh ${power_dir}PWR.sch ${bom_dir}pwr.prt

motor:
	mkdir -p ${motor_dir}/gerbers
	./makegerb.sh 4 ${motor_dir}motor.brd ${motor_dir}gerbers/motor
	./getprt.sh ${motor_dir}PWR.sch ${bom_dir}motor.prt
pwm:
	mkdir -p ${pwm_dir}/gerbers
	./makegerb.sh 5 ${pwm_dir}pwm-board.brd ${pwm_dir}gerbers/pwm
 	./getprt.sh ${pwm_dir}PWR.sch ${bom_dir}pwm-board.prt
jointio: 
	mkdir -p ${jointio_dir}/gerbers
	./makegerb.sh 4 ${jointio_dir}jointio.brd ${jointio_dir}gerbers/jointio
	./getprt.sh ${jointio_dir}PWR.sch ${bom_dir}jointio.prt
tile: 
	mkdir -p gerbers
	./makegerb.sh 4 ${power_dir}PWR1.brd ${power_dir}gerbers/shift-power 0 0
	./makegerb.sh 4 ${motor_dir}motor.brd ${motor_dir}gerbers/shift-motor -0.2 2.6
	./makegerb.sh 5 ${pwm_dir}pwm-board.brd ${pwm_dir}gerbers/shift-pwm 4.1 0.2
	./makegerb.sh 4 ${jointio_dir}jointio.brd ${jointio_dir}gerbers/shift-jointio 4 2.5

	cat ${power_dir}gerbers/shift-power-top.gbr ${motor_dir}gerbers/shift-motor-top.gbr ${pwm_dir}gerbers/shift-pwm-top.gbr ${jointio_dir}gerbers/shift-jointio-top.gbr > gerbers/top.gbr
	cat ${power_dir}gerbers/shift-power-bottom.gbr ${motor_dir}gerbers/shift-motor-bottom.gbr ${pwm_dir}gerbers/shift-pwm-bottom.gbr ${jointio_dir}gerbers/shift-jointio-bottom.gbr > gerbers/bottom.gbr
	cat ${power_dir}gerbers/shift-power-dim.gbr ${motor_dir}gerbers/shift-motor-dim.gbr ${pwm_dir}gerbers/shift-pwm-dim.gbr ${jointio_dir}gerbers/shift-jointio-dim.gbr > gerbers/dim.gbr
	cat ${power_dir}gerbers/shift-power-drill.dri ${motor_dir}gerbers/shift-motor-drill.dri ${pwm_dir}gerbers/shift-pwm-drill.dri ${jointio_dir}gerbers/shift-jointio-drill.dri > gerbers/drill.dri


.PHONY: clean

clean::
	-rm -f ${power_dir}gerbers/shift-power* 
	-rm -f ${motor_dir}gerbers/shift-motor*
	-rm -f ${pwm_dir}gerbers/shift-pwm*
	-rm -f ${jointio_dir}gerbers/shift-jointio*
	-rm -f ${power_dir}gerbers/power*
	-rm -f ${motor_dir}gerbers/motor*
	-rm -f ${pwm_dir}gerbers/pwm*
	-rm -f ${jointio_dir}gerbers/jointio*
	
	-rm -f ${power_dir}PWR.prt	
	-rm -f ${jointio.prt}jointio.prt	
	-rm -f ${motor_dir}motor.prt	
	-rm -f ${bom_dir}pwm.prt
