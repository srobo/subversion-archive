import colorsys
import sys
from PIL import Image

DEFAULTSATCUTOFF = 40

def get_min_sat(sathist):
    """
    Saturations below this value are white.
    """
    return DEFAULTSATCUTOFF

def get_cut_off(huehist):
    """
    Return a cut off between background white and a peak
    """
    return float(sum(huehist)) / 360

def get_colour(centre):
    colour = 0 #RED
    if centre >= 20 and centre < 85:
        colour = 1 #Yellow
    if centre >= 85 and centre < 158:
        colour = 2 #LGreen
    if centre >= 158 and centre < 180:
        colour = 3 #DGreen
    if centre >= 180 and centre < 207:
        colour = 4 #Cyan
    if centre >= 207 and centre < 230:
        colour = 5 #Blue
    if centre >= 230 and centre < 278:
        colour = 6 #Purple
    if centre >= 278 and centre <= 335:
        colour = 7 #Pink
    return colour


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
    

def get_image_pixels(filename):
    im = Image.open(sys.argv[1])
    return list(im.getdata())


def get_huehist(hsv, minsat):
    huecount = [0] * 360
    for i in range(0, len(hsv)):
        if hsv[i][1] >= minsat:
            curhue = int(hsv[i][0])
            huecount[curhue] = huecount[curhue] + 1
    return huecount
