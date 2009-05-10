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
    

im = Image.open(sys.argv[1]).resize((80, 60))
WIDTH, HEIGHT = im.size
out = Image.new("RGB", (WIDTH, HEIGHT))
dataout = out.load()
rgb = list(im.getdata())
hsv = [rgb_to_hsv(x[0], x[1], x[2]) for x in rgb]
hue = [x[0] for x in hsv]
sat = [x[1] for x in hsv]
val = [x[2] for x in hsv]

#map = []
#f = open("fullmap","r")
#for l in f.readlines():
#    v = (l.strip()).split()
#    map.append(int(float(v[1])) % 360)
#
#print map
#
#hue = [map[int(x)] for x in hue]

def get_data(huemin=0, huemax=360, satmin=0, satmax=100, valmin=0, valmax=100):
    print "face"
    global hues, sats, vals, huesats, im, out
    huecount = {}
    huesatcount = {}
    satcount = {}
    valcount = {}
    for x in range(0, 361):
        huecount[x] = 0
        huesatcount[x] = 0
    for x in range(0, 101):
        satcount[x] = 0
        valcount[x] = 0

    for i in range(0, len(hue)):
        curhue = int(hue[i])
        cursat = int(sat[i])
        curval = int(val[i])

        if cursat >= satmin and cursat <= satmax and \
            curval >= valmin and curval <= valmax and \
            curhue >= huemin and curhue <= huemax:
                dataout[i%WIDTH, i/WIDTH] =  (34, 255, 0)
        else:
                dataout[i%WIDTH, i/WIDTH] = rgb[i]


        if cursat >= satmin and cursat <= satmax and \
                curval >= valmin and curval <= valmax:
                    huecount[curhue] = huecount[curhue] + 1
                    huesatcount[curhue] += sat[i]
        if curhue >= huemin and curhue <= huemax and \
                curval >= valmin and curval <= valmax:
                    satcount[cursat] = satcount[cursat] + 1
        if curhue >= huemin and curhue <= huemax and \
                cursat >= satmin and cursat <= satmax:
                    valcount[curval] = valcount[curval] + 1
        
    hues = [huecount[x] for x in range(0, 360)]
    sats = [satcount[x] for x in range(0, 100)]
    vals = [valcount[x] for x in range(0, 100)]
    huesats = [huesatcount[x]/(huecount[x]+1) for x in range(0, 360)]
    
    subplot(322)
    cla()
    title("Saturation for hue %d - %d and val %d - %d" % (huemin, huemax, valmin,
        valmax))
    plot(sats)
    axis([-5, 105, 0, max(sats)])

    subplot(324)
    cla()
    title("Values for hue %d - %d and sat %d - %d" % (huemin, huemax, satmin,
        satmax))
    axis([-5, 105, 0, max(vals)])
    plot(vals)
    
    subplot(321)
    cla()
    title("Hues for val %d - %d and sat %d - %d. Average %d" % (valmin, valmax, satmin,
        satmax, sum(hues)/360))
    plot(hues)
    axis([-5, 370, 0, max(hues)]) #, max(sats), max(vals))])
    
    subplot(323)
    cla()
    plot(huesats)
    title("Average Saturation per hue")
    axis([-5, 370, 0, max(huesats)])

    subplot(325)
    cla()
    imshow(im)

    subplot(326)
    cla()
    imshow(out)


posdown = []
datadown = 0

def graph_press(event):
    print "Moo"
    global posdown, datadown
    if event.inaxes:
        posdown = event.inaxes.get_position()
        datadown = event.xdata

def graph_release(event):
    print "Poo"
    global posdown, datadown
    global hues, sats, vals, huesats
    if event.inaxes:
        posup = event.inaxes.get_position()
        dataup = event.xdata
        if posup == posdown:
            if posup[1] > 0.5 and \
            posup[0] == 0.125:
                #hue dragged
                get_data(huemin=min(dataup, datadown),
                         huemax=max(dataup, datadown))

            if posup[1] > 0.5 and posup[0] != 0.125:
                #Saturations
                get_data(satmin=min(dataup, datadown),
                         satmax=max(dataup, datadown))
            if posup[1] < 0.5 and posup[0] != 0.125:
                #values
                get_data(valmin=min(dataup, datadown),
                         valmax=max(dataup, datadown))

connect("button_press_event", graph_press)
connect("button_release_event", graph_release)

get_data()
show()
#savefig(sys.argv[1] + ".out.png")
