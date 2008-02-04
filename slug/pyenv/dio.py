from events import Event
import c2py
import logging

DIOADDRESS = 0x20
curdio = 0x0F #Bottom bits are inputs
try:
    c2py.writebyte(DIOADDRESS, curdio, 0)
except c2py.I2CError:
    logging.error("Error initilising DIO")

class DIOEvent(Event):
    def __init__(self, events):
        Event.__init__(self, diopoll)
        self.events = events

def setdio(self, bit, value):
    global curdio
    if bit < 4 or bit > 7:
        logging.error("Trying to set an invalid DIO pin.")
    else:
        if value == 0:
            curdio &= ~(1<<bit)
        else:
            curdio |= (1<<bit)
        c2py.writebyte(DIOADDRESS, curdio, 0)

def read():
    v = [None,None,None]
    pos = 0
    while True:
        try:
            v[pos] = c2py.readbyte(DIOADDRESS, 0) & 0xF
            pos = (pos + 1)%3

            if v[0] == v[1] and v[0] == v[2] and v[1] == v[2]:
                break
        except c2py.I2CError:
            logging.error("Error on I2C fetching DIO")

    return v[0]

def diopoll():
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
