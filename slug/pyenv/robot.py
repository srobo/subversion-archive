import vis

currentevent = None

def main(trampoline):
    print "Hello there, this is the start of the cheese"
    print "Going into a loop"

    cam = vis.Camera()

    while 1:
        #Wait for a frame
        vp = cam.vispoll()
        trampoline.addtask(vp)
        while True:
            print "Getting a frame then waiting for an event"
            cam.getframe()
            yield 2
            if currentevent.__class__ == vis.VISEvent:
                print "Got a frame"

                for blob in currentevent.blobs:
                    print blob.mass
                print "--------"
            trampoline.removetask(vp)
            
        #Now, wait for a number
        print "Waiting for a number (timeout = 0)"
        yield 0
        if currentevent.name == "Pin5":
            print "DIO Event"
        elif currentevent.name == "Blobs":
            print "Blob Event"
