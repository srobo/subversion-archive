from events import Event
import logging
import physics

class DIOEvent(Event):
    def __init__(self, events):
        self.events = events

class Dio:
    def read(self):
        return physics.World.bumpers

    def diopoll(self):
        self.last_read = self.read()
        yield None

        while 1:
            v = self.read()

            events = []

            for key, val in v.iteritems():
                if not key in self.last_read:
                    events.append((key, True))

            for key, val in self.last_read.iteritems():
                if not key in v:
                    events.append((key, False))

            self.last_read = v

            if len(events) > 0:
                yield DIOEvent(events)
            else:
                yield None


