from time import time

class Event:
    def __init__(self, source = None):
        self.source = None
        self.value = None

    def __eq__(self, obj):
        if self.source != None:
            return self.source == obj
        else:
            return NotImplemented

class TimeoutEvent(Event):
    def __init__(self, when):
        self.when = when
