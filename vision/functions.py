import colorsys
import sys
from PIL import Image

DEFAULTSATCUTOFF = 30
MINMASS = 25

def makemax(a, b, labels):
    if labels[a] == labels[b]:
        return
    elif labels[a] > labels[b]:
        #Change all instances of labels[b] to labels[a]
        change = labels[b]
        new = labels[a]
    else:
        change = labels[a]
        new = labels[b]

    for i in range(0, len(labels)):
        if labels[i] == change:
            labels[i] = labels[a]

def get_surrounding_labels(blobs, x, y, WIDTH, HEIGHT):
    toleft = None
    totop = None
    totopleft = None
    totopright = None
    curpos = y*WIDTH+x
    if x > 0:
        toleft = blobs[curpos - 1] 
        if y > 0:
            totopleft = blobs[curpos-(WIDTH+1)]
    if y > 0:
        totop = blobs[curpos - WIDTH]
        if x < WIDTH - 1:
            totopright = blobs[curpos-(WIDTH-1)]
    return toleft, totop, totopleft, totopright

def merge(labels, current, cells):
    for cell in cells:
        if cell != None:
            if cell != 0:
                makemax(current, cell, labels)

def label(data, minhue, maxhue, minsat, WIDTH, HEIGHT):
    labels = [0]
    blobs = [0] * WIDTH * HEIGHT #Initialise to all zeros
    topblobno = 1

    for y in range(0, HEIGHT):
        for x in range(0, WIDTH):
            curpos = y*WIDTH+x
            if data[curpos][1] > minsat and data[curpos][0] > minhue \
                    and data[curpos][0] < maxhue:
                toleft, totop, totopleft, totopright = \
                    get_surrounding_labels(blobs, x, y, WIDTH, HEIGHT)

                if toleft != None: #Look at the one to the left
                    if toleft != 0:
                        merge(labels, toleft, [totop, totopleft, totopright])
                        blobs[curpos] = toleft
                        continue

                if totopleft != None: #Look at the one to the left
                    if totopleft != 0:
                        merge(labels, totopleft, [totop, toleft, totopright])
                        blobs[curpos] = totopleft
                        continue

                if totop != None: #Look at the one to the left
                    if totop != 0:
                        merge(labels, totop, [toleft, totopleft, totopright])
                        blobs[curpos] = totop
                        continue

                if totopright != None: #Look at the one to the left
                    if totopright != 0:
                        merge(labels, totopright, [totop, totopleft, toleft])
                        blobs[curpos] = totopright
                        continue

                #If got this far it's a new blob
                labels.append(topblobno)        #New label
                blobs[curpos] = len(labels)-1   #It's the last one in the list as
                                                #it was just appended
                topblobno = topblobno + 1

    return blobs, labels

DEFAULTSATPEAK = 100
MINWHITEWIDTH = 2

def get_min_sat(sathist):
    """
    Saturations below this value are white.
    Go from 0 (fully saturated) right, until the count has gone above then
    below DEFAULTSATPEAK - if that was wider than MINWHITEWIDTH then presume
    that was the white peak.
    If don't find anything, return DEFAULTSATCUTOFF
    """
    whitepeakstart = 0
    for i in range(100):
        if whitepeakstart == 0:
            if sathist[i] > DEFAULTSATPEAK:
                whitepeakstart = i
        else:
            if sathist[i] < DEFAULTSATPEAK and \
                    i > whitepeakstart + MINWHITEWIDTH:
                return i
    return DEFAULTSATCUTOFF

def get_cut_off(huehist):
    """
    Return a cut off between background white and a peak. Just averaging, seems
    to be OK.
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

    return (h*359, s*99, v*99)
    

def get_image_pixels(filename):
    im = Image.open(sys.argv[1]).resize((80, 60))
    return list(im.getdata()), im.size


def get_hist(hsv, bins, getid, minid, minno):
    hist = [0] * bins
    for i in range(0, len(hsv)):
        if hsv[i][minid] >= minno:
            cur = int(hsv[i][getid])
            hist[cur] = hist[cur] + 1
    return hist
