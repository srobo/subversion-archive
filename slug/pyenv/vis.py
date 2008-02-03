import select
import os
import time
import subprocess
from events import Event

class VISEvent(Event):
    class Blob:
        def __init__(self, centrex, centrey, mass, colour):
            self.centrex = centrex
            self.centrey = centrey
            self.mass = mass
            self.colour = colour
    
    def __init__(self):
        Event.__init__(self, vispoll)
        self.blobs = []

    def addblob(self, centrex, centrey, mass, colour):
        self.blobs.append(self.Blob(centrex, centrey, mass, colour))

def vispoll():
    sp = subprocess.Popen("./testcam", stdout=subprocess.PIPE,
            stdin=subprocess.PIPE)
    fifo = sp.stdout.fileno()
    command = sp.stdin
    text = ""

    yield None #End of setup

    event = VISEvent()

    while True:
        command.write("\n")
        while True:
            if select.select([self.fifo], [], [], 0) == ([], [], []):
                #Nothing left to read
                break
            text += os.read(self.fifo, 1)

        if len(text) > 0:
            blocks = text.split("BLOBS\n")

            if len(blocks) > 1:
                text = blocks[-1]
                lastfull = blocks[-2]

                lines = lastfull.strip().split('\n')
                event = VISEvent()
                for line in lines:
                    if line != "":
                        info = line.split(",")
                        event.addblob(info[0], info[1], info[2], info[3])

                yield event

        #Haven't reached the end of a frame yet, so don't raise the event
        yield None
