from time import time

class Event:
    value = None

class TimeoutEvent(Event):
    def __init__(self):
        self.value = time()
