from events import Event
import c2py
from repeat import *
import logging

ADDRESS = 0x14

limits = []

curout = 0

# Get the identity from the board
JOINTIO_IDENTIFY = 0
# Set the outputs of the board
JOINTIO_OUTPUT = 1
# Read the board inputs in analogue form
JOINTIO_INPUT = 2
# Read the current digital output setting from the board.
JOINTIO_OUTPUT_READ = 3
# Read the inputs in digital form
JOINTIO_INPUT_DIG = 4


class DIOEvent(Event):
    def __init__(self, events):
        Event.__init__(self, diopoll)
        self.events = events

def setoutput(self, bit, value):
    global curout
    if bit > 4:
        logging.error("Trying to set an invalid DIO pin.")
    else:
        if value == 0:
            curout &= ~(1<<bit)
        else:
            curout |= (1<<bit)

        count = 0
        while count < MAXERR:
            setbyte(ADDRESS, JOINTIO_OUTPUT, curout)
            if getbyte(ADDRESS, JOINTIO_OUTPUT_READ) == curout:
                break

            count = count + 1

        if count == MAXERR:
            raise c2py.I2CError

def readinputs():
    val = getblock(ADDRESS, JOINTIO_INPUT, 16)
    bytes = [ord(x) for x in val]
    words = []
    for i in range(0, 8):
        words[i] = (bytes[2*i] << 8) & (bytes[2*i+1] & 0xFF)

    return words

def checkjointio():
    try:
        getbyte(ADDRESS, JOINTIO_OUTPUT_READ)
    except c2py.I2CError:
        return False
    return True

def iopoll():
    pass
    last_read = read()
    yield None

    while 1:
        v = read()
        diff = last_read ^ v
        last_read = v
        if diff:
            setbits = []
            for x in range(0, 4):
                if (diff & (1<<x)) != 0:
                    setbits.append( x )

            yield DIOEvent(setbits)
        else:
            yield None
