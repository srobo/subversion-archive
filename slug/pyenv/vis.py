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
        self.blobs = []

    def addblob(self, centrex, centrey, mass, colour):
        self.blobs.append(self.Blob(centrex, centrey, mass, colour))

class Camera:
    def __init__(self):
        self.sp = subprocess.Popen("./testcam", stdout=subprocess.PIPE,
                stdin=subprocess.PIPE)
        self.fifo = self.sp.stdout.fileno()
        self.command = self.sp.stdin
        self.streaming = False

    def getframe(self):
        self.command.write("\n")

    def vispoll(self):
        text = ""

        yield 0 #End of setup

        event = VISEvent()

        while True:
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
                        if line == "":
                            continue
                        info = line.split(",")
                        event.addblob(info[0], info[1], info[2], info[3])

                    if self.streaming:
                        self.getframe()

                    yield event

            #Haven't reached the end of a frame yet, so don't raise the event
            yield None
