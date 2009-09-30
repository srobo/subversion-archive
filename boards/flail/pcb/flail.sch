v 20081231 1
C 0 0 0 0 0 title-B.sym
C 3000 5100 1 0 0 resistor-1.sym
{
T 3300 5500 5 10 0 0 0 0 1
device=RESISTOR
T 3200 5400 5 10 1 1 0 0 1
refdes=R?
T 3000 5100 5 10 1 0 0 0 1
value=82k
T 3000 5100 5 10 1 1 0 0 1
footprint=0603
}
C 7600 4300 1 270 0 resistor-1.sym
{
T 8000 4000 5 10 0 0 270 0 1
device=RESISTOR
T 7900 4100 5 10 1 1 270 0 1
refdes=R?
T 7600 4300 5 10 1 1 0 0 1
value=2k
T 7600 4300 5 10 1 1 0 0 1
footprint=0603
}
C 7200 6500 1 90 0 capacitor-1.sym
{
T 6500 6700 5 10 0 0 90 0 1
device=CAPACITOR
T 6700 6700 5 10 1 1 90 0 1
refdes=C?
T 6300 6700 5 10 0 0 90 0 1
symversion=0.1
T 7200 6500 5 10 1 1 0 0 1
value=10nF
T 7200 6500 5 10 1 1 0 0 1
footprint=0603
}
C 9500 7300 1 270 0 capacitor-1.sym
{
T 10200 7100 5 10 0 0 270 0 1
device=CAPACITOR
T 10000 7100 5 10 1 1 270 0 1
refdes=C?
T 10400 7100 5 10 0 0 270 0 1
symversion=0.1
T 9500 7300 5 10 1 1 0 0 1
value=100nF
T 9500 7300 5 10 1 1 0 0 1
footprint=0603
}
C 8700 7300 1 270 0 capacitor-2.sym
{
T 9400 7100 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 9200 7100 5 10 1 1 270 0 1
refdes=C?
T 9600 7100 5 10 0 0 270 0 1
symversion=0.1
T 8700 7300 5 10 1 1 0 0 1
value=82uF
}
C 8900 4500 1 270 0 capacitor-2.sym
{
T 9600 4300 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 9400 4300 5 10 1 1 270 0 1
refdes=C?
T 9800 4300 5 10 0 0 270 0 1
symversion=0.1
T 8900 4500 5 10 1 1 0 0 1
value=330uF
}
C 7600 5700 1 0 0 inductor-1.sym
{
T 7800 6200 5 10 0 0 0 0 1
device=INDUCTOR
T 7800 6000 5 10 1 1 0 0 1
refdes=L?
T 7800 6400 5 10 0 0 0 0 1
symversion=0.1
T 7600 5700 5 10 1 1 0 0 1
value=33uH
}
C 6800 3300 1 90 0 schottky-1.sym
{
T 6128 3622 5 10 0 0 90 0 1
device=DIODE
T 6300 3600 5 10 1 0 90 0 1
refdes=D1
T 5968 3641 5 10 0 1 90 0 1
footprint=SOD80
T 6800 3300 5 10 1 1 0 0 1
value=SB520
}
C 7600 5500 1 270 0 resistor-variable-1.sym
{
T 8500 4700 5 10 0 0 270 0 1
device=VARIABLE_RESISTOR
T 8000 4900 5 10 1 1 270 0 1
refdes=R?
T 7600 5500 5 10 1 1 0 0 1
value=20k
}
C 4800 2700 1 0 0 ground.sym
N 7700 3000 7700 3400 4
N 2900 3000 2900 5200 4
N 2900 5200 3000 5200 4
N 7700 4300 7700 4600 4
N 6600 3300 6600 3000 4
N 9100 3600 9100 3000 4
N 9100 4500 9100 5000 4
N 8200 5000 11100 5000 4
N 8500 5800 9100 5800 4
N 9100 5800 9100 5000 4
N 7600 5800 7000 5800 4
N 6600 4200 7000 4200 4
N 7000 4200 7000 6500 4
N 7000 5700 6700 5700 4
N 8900 6400 8900 6200 4
N 8900 6200 10100 6200 4
N 10100 6200 10100 3000 4
N 9700 6400 9700 6200 4
N 7300 7300 9700 7300 4
N 7300 7300 7300 6100 4
N 7300 6100 6700 4900 4
N 8300 5000 8300 5300 4
N 6700 5300 8300 5300 4
N 7700 4500 6700 6100 4
N 3900 5200 4700 4900 4
N 4700 5300 3400 4700 4
N 3400 4700 3400 3000 4
N 4700 5700 2500 5600 4
N 2500 5600 2500 3000 4
N 2500 3000 10100 3000 4
N 4700 6100 3900 6100 4
N 3900 6100 3900 7600 4
N 3900 7600 7000 7600 4
N 7000 7600 7000 7400 4
N 9500 7300 9500 7700 4
T 9700 7700 9 10 1 0 0 0 1
Vin : 12V
T 11000 4700 9 10 1 0 0 0 1
Vout
T 4200 6300 9 10 1 0 0 0 1
BOOT
T 4300 5700 9 10 1 0 0 0 1
ENB
T 4300 5300 9 10 1 0 0 0 1
TSET
T 4300 4800 9 10 1 0 0 0 1
GND
T 6600 6300 9 10 1 0 0 0 1
VIN
T 6700 5800 9 10 1 0 0 0 1
LX
T 6600 5400 9 10 1 0 0 0 1
VBIAS
T 6700 4600 9 10 1 0 0 0 1
FB
C 4600 4400 1 0 0 A8498.sym
{
T 6400 5850 5 10 1 1 0 6 1
refdes=U?
T 5000 6050 5 10 0 0 0 0 1
device=A8498
T 5000 6250 5 10 0 0 0 0 1
footprint=A8498.fp
T 4600 4400 5 10 1 1 0 0 1
value=A8498
}
