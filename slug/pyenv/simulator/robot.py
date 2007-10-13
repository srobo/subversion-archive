from motor import *

currentevent = None
eventsource = None

def setmotors(a, b):
    setspeed(0, FORWARD, a)
    setspeed(1, FORWARD, b)


def main(trampoline):

    setmotors(100, 100)
    while 1:
        yield 0.5
        setmotors(100, 0)
        yield 0.3
        setmotors(100, 100)

