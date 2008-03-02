import colorsys
import sys
from PIL import Image
from pylab import *
from functions import *

MINWIDTH = 2 #Minimum width of a peak

#1. Load image as list of pixels
rgb, (WIDTH, HEIGHT) = get_image_pixels(sys.argv[1])

out = Image.new("RGB", (WIDTH, HEIGHT)) 
dataout = out.load()

#2. Convert to HSV
hsv = [rgb_to_hsv(x[0], x[1], x[2]) for x in rgb]

#3. Get a minimum saturation - below this is white
sathist = get_hist(hsv, 100, 1, [])

minsat = get_min_sat(sathist)

print "Minimum saturation: %d" % minsat

minval = get_min_val()

print "Minimum value is %d" % minval

for i in range(len(hsv)):
    if hsv[i][1] > minsat and hsv[i][2] > minval:
        dataout[i%WIDTH, i/WIDTH] =  rgb[i]
    else:
        dataout[i%WIDTH, i/WIDTH] = (34, 255, 0)

out.show()

#4. Generate a histogram of hue values
huehist = get_hist(hsv, 360, 0, [(1, minsat),(2, minval)])

plot(huehist)
show()
clf()

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

    blobs, labels = label(filtered, peak[0], peak[1], minsat, WIDTH, HEIGHT)
    
    geometry = {}

    for lab in [l for l in labels if l > 0]:
        geometry[lab] = [0, 0, 0]
    
    for y in range(0, HEIGHT):
        for x in range(0, WIDTH):
            curpos = y*WIDTH+x
            if blobs[curpos] > 0:
                lab = labels[blobs[curpos]]
                geometry[lab][0] += x
                geometry[lab][1] += y
                geometry[lab][2] += 1
    
    for l in geometry.itervalues():
        if l[2] > MINMASS:
            print "Colour %d, x=%d, y=%d, mass=%d" % (colour,
                                                    float(l[0]) / l[2],
                                                    float(l[1]) / l[2],
                                                    l[2])

