from motor import *
import dio
import vis
import physics

currentevent = None
eventsource = None

def forward(a, b):
    setspeed(0, FORWARD, a)
    setspeed(1, FORWARD, b)

def backward(a, b):
    setspeed(0, BACKWARD, a)
    setspeed(1, BACKWARD, b)

def turnright(speed):
    setspeed(1, FORWARD, speed)
    setspeed(0, BACKWARD, speed)

def turnleft(speed):
    setspeed(0, FORWARD, speed)
    setspeed(1, BACKWARD, speed)

LEFT = 0
RIGHT = 1

def main(trampoline):
    d = dio.Dio()
    dp = d.diopoll()
    c = vis.Camera()
    v = c.vispoll()
    trampoline.addtask(dp)
    trampoline.addtask(v)

    dir = LEFT

    while 1:
        if eventsource == v:
            forward(100, 100)
            trampoline.removetask(v)
            blob = currentevent.blobs[0]
            if blob.centrex > 0:
                dir = RIGHT
            else:
                dir = LEFT

            yield (float(blob.centrey) / 320) * 2

            trampoline.addtask(v)
        elif eventsource == dp:
            trampoline.removetask(v)

            while max([x for k, x in currentevent.events]):
                backward(30, 30)
                yield 1
            backward(20, 50)
            yield 1

            trampoline.addtask(v)
        else:
            turnright(20)
            yield 1
