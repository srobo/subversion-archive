all: power motor pwm jointio

power_dir=../power/pcb/trunk/
motor_dir=../power/pcb/trunk/
pwm_dir=../power/pcb/trunk/
jointio_dir=../power/pcb/trunk/

power: 
	mkdir -p ${power_dir}/gerbers
	./makegerb.sh ${power_dir}PWR1.brd ${power_dir}gerbers/power

motor:
	mkdir -p ${motor_dir}/gerbers
	./makegerb.sh ${motor_dir}motor.brd ${motor_dir}gerbers/power

pwm:
	mkdir -p ${pwm_dir}/gerbers
	./makegerb.sh ${pwm_dir}PWR1.brd ${pwm_dir}gerbers/power

jointio: 
	mkdir -p ${jointio_dir}/gerbers
	./makegerb.sh ${jointio_dir}PWR1.brd ${jointio_dir}gerbers/power

# tile: 
# 	mkdir -p ${}/gerbers
# 	./makegerb.sh ${power}PWR1.brd ${power}gerbers/power


# pwm: .config $(C_FILES) $(H_FILES)
# 	${CC} -mmcu=${CPU} -o $@ ${C_FILES} ${CFLAGS}

.PHONY: clean

clean::
	-rm -f pwm
