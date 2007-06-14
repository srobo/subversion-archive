import Image, ImageDraw
import sys

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
hl = int(sys.argv[2])
hh = int(sys.argv[3])
sl = int(sys.argv[4])
sh = int(sys.argv[5])
vl = int(sys.argv[6])
vh = int(sys.argv[7])


out = Image.new("RGB", (320, 240))

datain = im.load()
dataout = out.load()

for x in range(0, 320):
    for y in range(0, 240):
        rgb = datain[x, y]
        hsv = rgb_to_hsv(rgb[0], rgb[1], rgb[2])
        if (hsv[0] >= hl and hsv[0] <= hh) or \
           (hsv[1] >= sl and hsv[1] <= sh) or \
           (hsv[2] >= vl and hsv[2] <= vh):

            dataout[x, y] = (34, 255, 0)
        else:
            dataout[x, y] = rgb


out.save("coloured.png", "PNG")
