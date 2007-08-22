import colorsys
import sys
from PIL import Image
from functions import *

MINWIDTH = 2 #Minimum width of a peak

colours = [0,   #Red
           42,  #Yellow
           151, #Light green
           164, #Dark green
           195, #Cyan
           220, #Blue
           240, #Purple
           315] #Pink
           

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
