v 20091004 2
C 0 0 0 0 0 title-B.sym
C 3300 5200 1 0 0 resistor-1.sym
{
T 3600 5600 5 10 0 0 0 0 1
device=RESISTOR
T 3500 5500 5 10 1 1 0 0 1
refdes=RTSET
T 3500 5000 5 10 1 1 0 0 1
value=sr-r-82k-1206
T 3300 5200 5 10 0 1 0 0 1
footprint=1206
}
C 7900 5100 1 270 0 resistor-1.sym
{
T 8300 4800 5 10 0 0 270 0 1
device=RESISTOR
T 8200 4700 5 10 1 1 0 0 1
refdes=R2
T 8200 4400 5 10 1 1 0 0 1
value=sr-r-2k
}
C 7200 6500 1 90 0 capacitor-1.sym
{
T 6500 6700 5 10 0 0 90 0 1
device=CAPACITOR
T 6700 7000 5 10 1 1 180 0 1
refdes=CBOOT
T 6300 6700 5 10 0 0 90 0 1
symversion=0.1
T 6200 7100 5 10 1 1 0 0 1
value=sr-c-10n
}
C 9500 7300 1 270 0 capacitor-1.sym
{
T 10200 7100 5 10 0 0 270 0 1
device=CAPACITOR
T 9900 6800 5 10 1 1 0 0 1
refdes=CIN2
T 10400 7100 5 10 0 0 270 0 1
symversion=0.1
T 9700 6600 5 10 1 1 0 0 1
value=sr-c-100n
T 9700 7100 5 10 0 1 0 0 1
footprint=0603
}
C 8700 7300 1 270 0 capacitor-2.sym
{
T 9400 7100 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 8200 6800 5 10 1 1 0 0 1
refdes=CIN1
T 9600 7100 5 10 0 0 270 0 1
symversion=0.1
T 8200 6500 5 10 1 1 0 0 1
value=sr-c-82u
T 8700 7300 5 10 0 0 0 0 1
footprint=RADIAL_CAN 100
}
C 8900 4500 1 270 0 capacitor-2.sym
{
T 9600 4300 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 9300 4000 5 10 1 1 0 0 1
refdes=COUT
T 9800 4300 5 10 0 0 270 0 1
symversion=0.1
T 9000 3800 5 10 1 1 0 0 1
value=sr-c-330u
T 8900 4500 5 10 0 0 0 0 1
footprint=RADIAL_CAN 100
}
C 7600 5700 1 0 0 inductor-1.sym
{
T 7800 6200 5 10 0 0 0 0 1
device=INDUCTOR
T 7700 6000 5 10 1 1 0 0 1
refdes=L1
T 7800 6400 5 10 0 0 0 0 1
symversion=0.1
T 8000 5900 5 10 1 1 0 0 1
value=sr-l-33u
T 7600 5700 5 10 0 0 0 0 1
footprint=AXIAL_LAY 300
}
C 6800 3300 1 90 0 schottky-1.sym
{
T 6128 3622 5 10 0 0 90 0 1
device=DIODE
T 6300 3800 5 10 1 1 180 0 1
refdes=D1
T 5968 3641 5 10 0 1 90 0 1
footprint=DIODE_LAY_500
T 5500 3400 5 10 1 1 0 0 1
value=sr-d-1N5820
}
C 7400 4200 1 270 0 resistor-variable-1.sym
{
T 8300 3400 5 10 0 0 270 0 1
device=VARIABLE_RESISTOR
T 7800 3400 5 10 1 1 0 0 1
refdes=R1
T 7800 3200 5 10 1 1 0 0 1
value=sr-pot-20k
}
C 4800 2700 1 0 0 ground.sym
N 6600 3300 6600 3000 4
N 9100 3600 9100 3000 4
N 9100 4500 9100 5800 4
N 8500 5800 9100 5800 4
N 7600 5800 7000 5800 4
N 6600 4200 7000 4200 4
N 7000 4200 7000 6500 4
N 7000 5700 6700 5700 4
N 8900 6400 8900 6200 4
N 8900 6200 10100 6200 4
N 10100 6200 10100 3000 4
N 9700 6400 9700 6200 4
N 7300 7300 9700 7300 4
N 6700 5300 8500 5300 4
N 2800 5300 2800 3000 4
N 4700 5700 2500 5700 4
N 2500 5700 2500 3000 4
N 2500 3000 12300 3000 4
N 4700 6100 3900 6100 4
N 3900 6100 3900 7600 4
N 3900 7600 7000 7600 4
N 7000 7600 7000 7400 4
N 9500 7300 9500 7700 4
T 9700 7700 9 10 1 0 0 0 1
Vin : 12V
T 12600 3400 9 10 1 0 0 0 1
Vout
C 4600 4400 1 0 0 A8498.sym
{
T 6200 6550 5 10 1 1 0 6 1
refdes=U1
T 5000 6050 5 10 0 0 0 0 1
device=A8498
T 5000 6250 5 10 0 0 0 0 1
footprint=A8498.fp
T 5000 4300 5 10 1 1 0 0 1
value=sr-ic-a8498
}
N 3400 4900 3400 3000 4
N 6700 6100 7300 6100 4
N 9500 7600 11500 7600 4
N 10600 7600 10600 6300 4
C 10500 6300 1 270 0 resistor-1.sym
{
T 10900 6000 5 10 0 0 270 0 1
device=RESISTOR
T 10700 5800 5 10 1 1 0 0 1
refdes=RPWR
T 10700 5600 5 10 1 1 0 0 1
value=sr-r-1k-1206
T 10500 6300 5 10 0 1 0 0 1
footprint=1206
}
C 10500 4500 1 270 0 led-2.sym
{
T 10400 3700 5 10 1 1 0 0 1
refdes=sr-led-green-plcc2
T 11100 4400 5 10 0 0 270 0 1
device=LED
T 10500 4500 5 10 0 0 0 0 1
footprint=plcc
}
N 10600 5400 10600 4500 4
N 10600 3600 10600 3000 4
N 4700 5300 4200 5300 4
N 3300 5300 2800 5300 4
N 3400 4900 4700 4900 4
N 7300 6100 7300 7300 4
N 6700 4900 7700 4900 4
C 14000 7600 1 180 0 connector2-1.sym
{
T 13800 6600 5 10 0 0 180 0 1
device=CONNECTOR_2
T 14000 6800 5 10 1 1 180 0 1
refdes=CONN_In
T 12700 7700 5 10 1 1 0 0 1
value=sr-cn-camcon2w
}
C 14000 3500 1 180 0 connector2-1.sym
{
T 13800 2500 5 10 0 0 180 0 1
device=CONNECTOR_2
T 14000 2700 5 10 1 1 180 0 1
refdes=CONN_Out
T 14000 3500 5 10 0 0 0 0 1
footprint=CONNECTOR 1 2
T 12700 3600 5 10 1 1 0 0 1
value=sr-cn-camcon2w
}
N 12300 7400 11500 7400 4
N 11500 7400 11500 7600 4
N 12300 7100 12000 7100 4
N 12000 3000 12000 7100 4
N 12300 3300 12300 5100 4
C 8300 4400 1 270 0 capacitor-1.sym
{
T 9000 4200 5 10 0 0 270 0 1
device=CAPACITOR
T 8600 4100 5 10 1 1 0 0 1
refdes=CBIAS
T 9200 4200 5 10 0 0 270 0 1
symversion=0.1
T 8300 4400 5 10 0 0 0 0 1
footprint=0603
T 8200 3800 5 10 1 1 0 0 1
value=sr-c-10n
}
N 8500 4400 8500 5300 4
N 8500 3500 8500 3000 4
N 7500 3300 7500 3000 4
N 8000 3700 8000 4200 4
N 8000 4200 7700 4200 4
N 7700 4200 7700 4900 4
N 8000 5100 12300 5100 4
