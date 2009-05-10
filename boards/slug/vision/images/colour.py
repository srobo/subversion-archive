import Image, ImageDraw
import sys
from conv import rgb_to_hsv

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
