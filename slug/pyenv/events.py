from time import time

class Event:
    value = None

    def type(self):
        return self.__class__

class TimeoutEvent(Event):
    def __init__(self):
        self.value = time()
