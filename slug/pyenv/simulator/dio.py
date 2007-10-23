from events import Event
import physics

class DIOEvent(Event):
    def __init__(self, events):
        super(DIOEvent, self).__init__(diopoll)
        self.events = events

def diopoll():
    last_read = physics.World.bumpers
    yield None

    while 1:
        v = physics.World.bumpers

        pins = {}

        for key, val in v.iteritems():
            if not key in last_read:
                pins[key] = val

        for key, val in last_read.iteritems():
            if not key in v:
                pins[key] = val

        last_read = v

        if len(pins) > 0:
            yield DIOEvent(events)
        else:
            yield None
