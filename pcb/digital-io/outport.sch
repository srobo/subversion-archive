v 20060906 1
C 65200 52800 1 0 0 vdd-1.sym
C 65600 50600 1 180 0 vss-1.sym
C 68300 51200 1 0 1 connector2-1.sym
{
T 68100 52200 5 10 0 0 0 6 1
device=CAGECLAMP
T 68300 52000 5 10 1 1 0 6 1
refdes=CONN?
T 68300 51200 5 10 0 0 0 0 1
footprint=cageclamp
}
C 63300 51600 1 0 0 in-1.sym
{
T 63300 51900 5 10 0 0 0 0 1
device=INPUT
T 63300 51900 5 10 1 1 0 0 1
refdes=output
}
C 64900 52600 1 180 1 pmos-3.sym
{
T 65500 52100 5 10 0 0 180 6 1
device=PMOS_TRANSISTOR
T 65600 52000 5 10 1 1 180 6 1
refdes=Q?
}
C 64900 50800 1 0 0 nmos-3.sym
{
T 65500 51300 5 10 0 0 0 0 1
device=NMOS_TRANSISTOR
T 65600 51400 5 10 1 1 0 0 1
refdes=Q?
}
N 65400 51800 65400 51600 4
N 65400 51700 66600 51700 4
N 64900 52400 64600 52400 4
N 64600 52400 64600 51000 4
N 64600 51000 64900 51000 4
N 63900 51700 64600 51700 4
N 66600 51400 66200 51400 4
N 66200 51400 66200 51700 4
N 65400 52800 65400 52600 4
N 65400 50800 65400 50600 4
