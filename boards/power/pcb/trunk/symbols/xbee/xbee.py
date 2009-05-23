#!/usr/bin/env python
import sys
sys.path.append('/home/tom/project/pcbfoot/')
import dil_ic
import padgen
from padgen import mm
pins = dil_ic.dilic('xbee', mm(22), mm(2), 20, mm(0.8), 0, 0)

#xbee details
#long faces
base = 24.52 
side = 22
#square sides of corner triangles
corner_base = 7.59
corner_height = 6.53

a= base/2
b= side/2

poly =[[-a,-b],[-a,b],[a,b],[a,-b],[a-corner_base,-b-corner_height],[-a+corner_base,-b-corner_height]]

for i in range(0,len(poly)):
    pins.add(padgen.line(mm(poly[i][0]),mm(poly[i][1]),mm(poly[i-1][0]),mm(poly[i-1][1]),1000))
 
f = open(sys.argv[1],'w')
f.write(pins.generate())
