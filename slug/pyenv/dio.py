from events import Event
import c2py
import logging

DIOADDRESS = 0x20

class DIOEvent(Event):
    def __init__(self, bits):
        self.bits = bits

class Dio:
    def __init__(self):
        self.curdio = 0x0F #Bottom bits are inputs
        print "Writing to %x, %x" % (DIOADDRESS, self.curdio)
        c2py.writebyte(DIOADDRESS, self.curdio, 0)

    def setdio(self, bit, value):
        if bit < 4 or bit > 7:
            logging.error("Trying to set an invalid DIO pin.")
        else:
            if value == 0:
                self.curdio &= ~(1<<bit)
            else:
                self.curdio |= (1<<bit)
            print "%x" % self.curdio
            c2py.writebyte(DIOADDRESS, self.curdio, 0)

    def diopoll(self):
        yield None
        while 1:
            tmpdio = c2py.readbyte(DIOADDRESS, 0)
            print "Read: %x" % tmpdio
            diff = tmpdio ^ self.curdio
            print "Diff: %x" % diff
            if diff:
                setbits = []
                for x in range(0, 4):
                    if (diff & (1<<x)) != 0:
                        setbits.append(x)

                yield DIOEvent(setbits)

            else:
                yield None
