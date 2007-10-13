from events import Event
import c2py
import logging

DIOADDRESS = 0x20

class DIOEvent(Event):
    def __init__(self, bits):
        self.bits = bits

def getbit(val,n):
    if (1<<n)&val:
        return 1
    else:
        return 0

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

    def read(self):
        v = [None,None,None]
        pos = 0

        while True:
            try:
                v[pos] = c2py.readbyte(DIOADDRESS, 0) & 0xF
                pos = (pos + 1)%3

                if v[0] == v[1] and v[0] == v[2] and v[1] == v[2]:
                    break
            except c2py.I2CError:
                print "Error on I2C"
                pass

        return v[0]

    def diopoll(self):
        self.last_read = self.read()
        print "diopoll init"
        yield None

        while 1:
            v = self.read()
            diff = self.last_read ^ v
            self.last_read = v
            if diff:
                setbits = []
                for x in range(0, 4):
                    if (diff & (1<<x)) != 0:
                        print getbit(v,x)
                        setbits.append( (x,getbit(v,x)) )

                yield DIOEvent(setbits)

            else:
                yield None


