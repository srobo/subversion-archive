from motor import *
import dio

currentevent = None
eventsource = None

def forward(a, b):
    setspeed(0, FORWARD, a)
    setspeed(1, FORWARD, b)

def backward(a, b):
    setspeed(0, BACKWARD, a)
    setspeed(1, BACKWARD, b)

def main(trampoline):
    d = dio.Dio()
    dp = d.diopoll()
    trampoline.addtask(dp)

    forward(100, 100)
    while 1:
        yield 0
        if eventsource == dp:
            while eventsource != None:
                backward(100, 100)
                yield 0.3
            forward(100, 100)
