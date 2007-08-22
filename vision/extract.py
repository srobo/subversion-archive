import colorsys
import sys
from PIL import Image
from functions import *

MINWIDTH = 2 #Minimum width of a peak

#1. Load image as list of pixels
rgb = get_image_pixels(sys.argv[1])

#2. Convert to HSV
hsv = [rgb_to_hsv(x[0], x[1], x[2]) for x in rgb]

#3. Get a minimum saturation - below this is white
minsat = get_min_sat([])

#4. Generate a histogram of hue values
huehist = get_huehist(hsv, minsat)

#5. Find a minimum useful peak height
huecutoff = get_cut_off(huehist)

#6. Find peaks that are greater than minwidth wide that go above the huecutoff

curpeakstart = 0 #Start hue point
curpeakmass = 0 #Track the mass of the hue

OUT = 0
IN = 1
peakstate = OUT #As go through hue values, in a peak or out of it

peaks = []

for i in range(0, 360):
    if peakstate == OUT:
        #Wait to find a peak start
        if huehist[i] > huecutoff:
            peakstate = IN
            curpeakstart = i
            curpeakmass += huehist[i]
    else:
        if huehist[i] < huecutoff:
            #Peak ended
            peakstate = OUT
            if i - curpeakstart > MINWIDTH:
                peaks.append((curpeakstart, i, curpeakmass))
            curpeakmass = 0
        else:
            curpeakmass += huehist[i]

#Go through each peak, pulling out the colour and then identifying blobs
for peak in peaks:
    centre = (peak[0] + peak[1]) / 2 #Centre hue value in range. TODO: Bias by
                                     #mass?
    colour = get_colour(centre)      #Range 0 - 7. TODO: Make this function
                                     #tweakable
    
    #Go through image, labelling blobs for this colour
    #TODO: Best to go through image once, checking for all the colours at once?
    filtered = [x for x in hsv]
    blobs, labels = label(filtered, peak[0], peak[1], minsat, 320, 240)
    
    geometry = {}

    for label in [l for l in labels if l > 0]:
        geometry[label] = [0, 0, 0]

    
    for y in range(0, 240):
        for x in range(0, 320):
            curpos = y*320+x
            if blobs[curpos] > 0:
                label = labels[blobs[curpos]]
                geometry[label][0] += x
                geometry[label][1] += y
                geometry[label][2] += 1
    
    for l in geometry.itervalues():
        print "Colour %d, x=%d, y=%d, mass=%d" % (colour,
                                                  float(l[0]) / l[2],
                                                  float(l[1]) / l[2],
                                                  l[2])

