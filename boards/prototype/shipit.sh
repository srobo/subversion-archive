#/bin/bash
rm -rf ship
mkdir ship
cd ship
mkdir power motor pwm jointio
cd ..
cp ../power/pcb/trunk/gerbers/power-*.{gbr,drd} ship/power/
cp ../motor/pcb/trunk/gerbers/motor-*.{gbr,drd} ship/motor/
cp ../pwm/pcb/trunk/gerbers/pwm-*.{gbr,drd} ship/pwm/
cp ../jointio/pcb/trunk/gerbers/jointio-*.{gbr,drd} ship/jointio/

zip -r ship ship