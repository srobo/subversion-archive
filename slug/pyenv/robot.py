from dio import diopoll
from vis import Camera

currentevent = None

def main(trampoline):
    print "Hello there, this is the start of the cheese"
    print "Going into a loop"

    cam = Camera()

    while 1:
        #Wait for a frame
        vp = cam.vispoll()
        trampoline.addtask("Blobs", vp)
        trampoline.addtask("Pin5", diopoll())
        while True:
            print "Trying to get a frame - this should timeout"
            yield 2
            print "Event raised: " + str(currentevent.__class__)

            print "Getting a frame then waiting for an event"
            cam.getframe()
            yield 2
            print currentevent.__class__
            if currentevent.__class__ == "VisEvent":
                print "Got a frame"
                for blob in currentevent.blobs:
                    print blob.mass
                print "--------"
            print "This should wait for 3s"
            yield 3
            print "3s Waiting over"
        #Now, wait for a number
        trampoline.addtask("Pin5", diopoll( 5 ))
        print "Waiting for a number (timeout = 0)"
        yield 0
        if currentevent.name == "Pin5":
            print "DIO Event"
        elif currentevent.name == "Blobs":
            print "Blob Event"
