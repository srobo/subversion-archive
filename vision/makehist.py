import colorsys
import sys
from PIL import Image
from pylab import *

def rgb_to_hsv(r, g, b):
    r = float(r)/255
    g = float(g)/255
    b = float(b)/255
	
    minval = min(r, g, b)
    maxval = max(r, g, b)
    delta = maxval - minval

    v = maxval
    if delta == 0:
        h = 0
        s = 0
    else:
        s = delta / maxval
        del_r = (((maxval - r) / 6) + (delta / 2)) / delta
        del_g = (((maxval - g) / 6) + (delta / 2)) / delta
        del_b = (((maxval - b) / 6) + (delta / 2)) / delta

        if r == maxval:
            h = del_b - del_g
        elif g == maxval:
            h = (float(1)/3) + del_r - del_b
        elif b == maxval:
            h = (float(2)/3) + del_g - del_r

        if h < 0:
            h = h + 1
        if h > 1:
            h = h - 1

    return (h*360, s*100, v*100)
    

im = Image.open(sys.argv[1])
rgb = list(im.getdata())
hsv = [rgb_to_hsv(x[0], x[1], x[2]) for x in rgb]
hue = [x[0] for x in hsv]
sat = [x[1] for x in hsv]
val = [x[2] for x in hsv]
huecount = {}
satcount = {}
valcount = {}
for x in range(0, 360):
    huecount[x] = 0
for x in range(0, 100):
    satcount[x] = 0
    valcount[x] = 0

for x in hue:
    huecount[int(x)] = huecount[int(x)] + 1
for x in sat:
    satcount[int(x)] = satcount[int(x)] + 1
for x in val:
    valcount[int(x)] = valcount[int(x)] + 1

hues = [huecount[x] for x in range(0, 360)]
sats = [satcount[x] for x in range(0, 100)]
vals = [valcount[x] for x in range(0, 100)]

plot(hues)
plot(sats)
plot(vals)

show()
