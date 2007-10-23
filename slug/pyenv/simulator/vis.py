from events import Event
import physics

FRAMEWIDTH = 352
FRAMEHEIGHT = 288

class VISEvent(Event):
    class Blob:
        def __init__(self, centrex, centrey, mass, colour):
            self.centrex = centrex
            self.centrey = centrey
            self.mass = mass
            self.colour = colour
    
    def __init__(self):
        super(VISEvent, self).__init__(vispoll)
        self.blobs = []

    def addblob(self, centrex, centrey, mass, colour):
        self.blobs.append(self.Blob(centrex, centrey, mass, colour))

def vispoll():
    yield None #End of setup

    while True:
        blobs = physics.World.blobs
        if len(blobs) > 0:
            event = VISEvent()
            for blob in blobs:
                event.addblob(int(blob[0]*FRAMEWIDTH),
                                int(blob[1]*FRAMEHEIGHT),
                                int(blob[2]),
                                0)
            yield event
        else:
            yield None
