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

    while 1:
        setspeed(0, FORWARD, 20)
        setspeed(1, BACKWARD, 20)
        yield 1
        forward(100, 100)
        yield 1
