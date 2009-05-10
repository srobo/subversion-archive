import Image

im = Image.open("blobs.bmp")

WIDTH, HEIGHT = im.size

data = im.getdata()

labels = [0]

blobs = [0] * WIDTH * HEIGHT #Initialise to all zeros

topblobno = 1

def makemax(a, b):
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

for y in range(0, HEIGHT):
    for x in range(0, WIDTH):
        curpos = y*WIDTH+x
        if data[curpos] == (255, 255, 255):
            #White
            pass
        else:
            #Black
            toleft = None
            totop = None
            totopleft = None
            totopright = None
            if x > 0:
                toleft = blobs[curpos - 1] 
                if y > 0:
                    totopleft = blobs[curpos-(WIDTH+1)]
            if y > 0:
                totop = blobs[curpos - WIDTH]
                if x < WIDTH - 1:
                    totopright = blobs[curpos-(WIDTH-1)]

            if toleft != None: #Look at the one to the left
                if toleft != 0:
                    current = toleft
                    
                    #Check for merges
                    if totop != None:
                        if totop != 0:
                            makemax(current, totop)
                    if totopleft != None:
                        if totopleft != 0:
                            makemax(current, totopleft)
                    if totopright != None:
                        if totopright != 0:
                            makemax(current, totopright)

                    blobs[curpos] = current
                    continue

            if totopleft != None: #Look at the one to the left
                if totopleft != 0:
                    current = totopleft
                    
                    #Check for merges
                    if toleft != None:
                        if toleft != 0:
                            makemax(current, toleft)
                    if totop != None:
                        if totop != 0:
                            makemax(current, totop)
                    if totopright != None:
                        if totopright != 0:
                            makemax(current, totopright)

                    blobs[curpos] = current
                    continue

            if totop != None: #Look at the one to the left
                if totop != 0:
                    current = totop
                    
                    #Check for merges
                    if toleft != None:
                        if toleft != 0:
                            makemax(current, toleft)
                    if totopleft != None:
                        if totopleft != 0:
                            makemax(current, totopleft)
                    if totopright != None:
                        if totopright != 0:
                            makemax(current, totopright)

                    blobs[curpos] = current
                    continue


            if totopright != None: #Look at the one to the left
                if totopright != 0:
                    current = totopright
                    
                    #Check for merges
                    if toleft != None:
                        if toleft != 0:
                            makemax(current, toleft)
                    if totop != None:
                        if totop != 0:
                            makemax(current, totop)
                    if totopleft != None:
                        if totopleft != 0:
                            makemax(current, totopleft)

                    blobs[curpos] = current
                    continue

            #If got this far it's a new blob
            labels.append(topblobno)        #New label
            blobs[curpos] = len(labels)-1   #It's the last one in the list as
                                            #it was just appended
            topblobno = topblobno + 1

print "%d blobs found." % topblobno
i = 0
for label in labels:
    print i, label
    i = i + 1

out = Image.new("RGB", (WIDTH, HEIGHT))

col = [(labels[i]*10 % 255, labels[i] * 20 % 255, labels[i] * 30 % 255) for i in blobs]

out.putdata(col)

out.save("out.bmp")
