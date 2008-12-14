v 20060906 1
C 72300 48500 1 0 0 resistor-2.sym
{
T 72700 48850 5 10 0 0 0 0 1
device=RESISTOR
T 72600 48800 5 10 1 1 0 0 1
refdes=R201
T 72600 48300 5 10 1 1 0 0 1
value=10k
T 72300 48500 5 10 0 0 0 0 1
footprint=1206_reflow_solder
}
C 75600 48100 1 0 1 connector2-1.sym
{
T 75400 49100 5 10 0 0 0 6 1
device=CAGECLAMP
T 75600 48900 5 10 1 1 0 6 1
refdes=CONN201
T 75600 48100 5 10 0 0 0 0 1
footprint=cageclamp
}
C 70100 50300 1 0 0 vdd-1.sym
C 72700 49900 1 180 0 vss-1.sym
N 73900 48600 73200 48600 4
N 73600 48300 73600 48600 4
C 70400 48500 1 0 1 out-1.sym
{
T 70400 48800 5 10 0 0 0 6 1
device=OUTPUT
T 70400 48800 5 10 1 1 0 6 1
refdes=input
}
N 72300 50100 72500 50100 4
N 72500 50100 72500 49900 4
N 70300 50300 70300 50100 4
N 70300 50100 70500 50100 4
N 70400 48600 72300 48600 4
N 71400 49800 71400 48600 4
N 73600 48300 73900 48300 4
C 72400 51200 1 180 0 bat754s.sym
{
T 72000 50800 5 10 0 0 180 0 1
device=BAT754S
T 71500 50500 5 10 1 1 180 0 1
refdes=Z201
T 72000 51100 5 10 0 1 180 0 1
footprint=SOT-23_Transistor
}