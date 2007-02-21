v 20060906 1
C 67800 47400 0 0 0 title-A4.sym
C 70000 49300 1 0 1 rj11-custom.sym
{
T 70000 52200 5 10 0 0 0 6 1
device=RJ45
T 68900 51000 5 10 1 1 0 0 1
refdes=CONN101
T 67800 51600 5 10 0 1 0 6 1
footprint=rj11-6p4c
}
C 75400 53300 1 0 0 sub-inport.sym
{
T 76460 53400 5 10 1 1 0 0 1
source=sub-inport.sch
T 75500 53300 5 10 0 0 0 0 1
device=none
T 76300 53600 5 10 1 1 0 0 1
refdes=S101
}
C 75400 52800 1 0 0 sub-inport.sym
{
T 76460 52900 5 10 1 1 0 0 1
source=sub-inport.sch
T 75500 52800 5 10 0 0 0 0 1
device=none
T 76300 53100 5 10 1 1 0 0 1
refdes=S102
}
C 75400 52300 1 0 0 sub-inport.sym
{
T 76460 52400 5 10 1 1 0 0 1
source=sub-inport.sch
T 75500 52300 5 10 0 0 0 0 1
device=none
T 76300 52600 5 10 1 1 0 0 1
refdes=S103
}
C 75400 51800 1 0 0 sub-inport.sym
{
T 76460 51900 5 10 1 1 0 0 1
source=sub-inport.sch
T 75500 51800 5 10 0 0 0 0 1
device=none
T 76300 52100 5 10 1 1 0 0 1
refdes=S104
}
C 75400 51300 1 0 0 sub-outport.sym
{
T 76460 51400 5 10 1 1 0 0 1
source=sub-outport.sch
T 75500 51300 5 10 0 0 0 0 1
device=none
T 76300 51600 5 10 1 1 0 0 1
refdes=S105
}
C 75400 50800 1 0 0 sub-outport.sym
{
T 76460 50900 5 10 1 1 0 0 1
source=sub-outport.sch
T 75500 50800 5 10 0 0 0 0 1
device=none
T 76300 51100 5 10 1 1 0 0 1
refdes=S106
}
C 75400 50300 1 0 0 sub-outport.sym
{
T 76460 50400 5 10 1 1 0 0 1
source=sub-outport.sch
T 75500 50300 5 10 0 0 0 0 1
device=none
T 76300 50600 5 10 1 1 0 0 1
refdes=S107
}
C 75400 49800 1 0 0 sub-outport.sym
{
T 76460 49900 5 10 1 1 0 0 1
source=sub-outport.sch
T 75500 49800 5 10 0 0 0 0 1
device=none
T 76300 50100 5 10 1 1 0 0 1
refdes=S108
}
C 70600 52500 1 0 0 sub-jumpers.sym
{
T 70600 52500 5 10 1 1 0 0 1
source=sub-jumpers.sch
T 70600 54400 5 10 1 1 0 0 1
refdes=S109
T 70600 52470 5 10 0 1 0 0 1
device=none
}
N 69900 50700 70200 50700 4
N 70200 50700 70200 55000 4
N 70200 55000 73900 55000 4
N 73900 55000 73900 54800 4
N 69900 50100 70200 50100 4
N 70200 50100 70200 49000 4
N 70200 49000 73900 49000 4
N 73900 49000 73900 49200 4
N 69900 50500 72600 50500 4
N 69900 50300 72600 50300 4
N 72200 54000 72600 54000 4
N 72200 53500 72600 53500 4
N 72200 53000 72600 53000 4
N 74600 53500 75500 53500 4
N 74600 53000 75500 53000 4
N 74600 52500 75500 52500 4
N 74600 52000 75500 52000 4
N 74600 51500 75500 51500 4
N 74600 51000 75500 51000 4
N 74600 50500 75500 50500 4
C 70800 55100 1 0 0 vdd-1.sym
C 71200 48900 1 180 0 vss-1.sym
N 71000 48900 71000 49000 4
N 71000 55100 71000 55000 4
C 72500 49100 1 0 0 pcf8574.sym
{
T 74700 54500 5 10 1 1 0 6 1
refdes=U101
T 72900 54900 5 10 0 0 0 0 1
device=PCF8574
T 72900 55100 5 10 0 0 0 0 1
footprint=soic-16w-lpads
}
N 74600 50000 75500 50000 4
C 68700 54600 1 0 0 vdd-1.sym
C 69100 53700 1 180 0 vss-1.sym
C 68700 54600 1 270 0 capacitor-4.sym
{
T 69800 54400 5 10 0 0 270 0 1
device=POLARIZED_CAPACITOR
T 69200 54400 5 10 1 1 270 0 1
refdes=C1
T 69400 54400 5 10 0 0 270 0 1
symversion=0.1
T 68700 54600 5 10 0 0 0 0 1
footprint=quickcapfoot-2.5mm
}
