v 20060906 1
C 65200 53000 1 0 0 vdd-1.sym
C 65600 50400 1 180 0 vss-1.sym
C 68300 51200 1 0 1 connector2-1.sym
{
T 68100 52200 5 10 0 0 0 6 1
device=CAGECLAMP
T 68300 52000 5 10 1 1 0 6 1
refdes=CONN301
T 68300 51200 5 10 0 0 0 0 1
footprint=cageclamp
}
C 63300 51600 1 0 0 in-1.sym
{
T 63300 51900 5 10 0 0 0 0 1
device=INPUT
T 63300 51900 5 10 1 1 0 0 1
refdes=output301
}
N 65400 51700 66600 51700 4
N 64900 52600 64600 52600 4
N 64600 52600 64600 50800 4
N 64600 50800 64900 50800 4
N 63900 51700 64600 51700 4
N 66600 51400 66200 51400 4
N 66200 51400 66200 51700 4
N 65400 50600 65400 50400 4
N 65400 53000 65400 52800 4
N 65400 51400 65400 52000 4
C 64900 50600 1 0 0 nmos.sym
{
T 65400 52100 5 10 0 0 0 0 1
device=BSS670S2L
T 65600 51200 5 10 1 1 0 0 1
refdes=Q301
T 64900 50600 5 10 0 1 0 0 1
footprint=SOT-23_Transistor
}
C 64900 52800 1 180 1 pmos.sym
{
T 65600 51700 5 10 0 0 180 6 1
device=ZXM61P02F
T 65600 52200 5 10 1 1 180 6 1
refdes=Q302
T 66500 52800 5 10 0 1 180 6 1
footprint=SOT-23_Transistor
}
