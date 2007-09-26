from events import Event

class DIOEvent(Event):
    def __init__(self, value):
        self.value = value

def diopoll():
    yield
    while 1:
        try:
            f = open("/tmp/tst", "rt")
            number = int(f.read())
            f.close()
            yield DIOEvent(number)
        except IOError:
            yield None
