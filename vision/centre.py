import Image

im = Image.open("blobs.bmp")

WIDTH, HEIGHT = im.size

data = im.getdata()

blobs = [0] * WIDTH * HEIGHT #Initialise to all zeros

topblobno = 1

for y in range(0, HEIGHT):
    for x in range(0, WIDTH):
        curpos = y*WIDTH+x
        if data[curpos] == (255, 255, 255):
            #White
            pass
        else:
            #Black
            if x > 0: #Look at the one to the left
                if blobs[curpos-1] > 0:
                    blobs[curpos] = blobs[curpos-1]
                    break
            if y > 0: #Look at the one above
                if blobs[curpos-WIDTH] > 0:
                    blobs[curpos] = blobs[curpos-WIDTH]
                    break
            if x > 0 and y > 0: #Look top-left
                if blobs[curpos-(WIDTH+1)] > 0:
                    blobs[curpos] = blobs[curpos-(WIDTH+1)]
                    break
            if x < WIDTH - 1 and y > 0: #Look top-right
                if blobs[curpos-(WIDTH-1)] > 0:
                    blobs[curpos] = blobs[curpos-(WIDTH-1)]
                    break
            
            #If got this far it's a new blob
            blobs[curpos] = topblobno
            topblobno = topblobno + 1

print "%d blobs found." % topblobno

out = Image.new("RGB", (WIDTH, HEIGHT))

col = [(i*20 % 255, i * 20 % 255, i * 20 % 255) for i in blobs]

out.putdata(col)

out.save("out.bmp")
