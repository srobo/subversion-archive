from events import Event
import time

timeout = "timeout"

class TimeoutEvent(Event):
    def __init__(self, when):
        Event.__init__(self, timeout)
        self.when = when
    def __str__(self):
        return "TimeoutEvent"

def time_poll(t):
    start = time.time()
    while True:
        if (time.time() - start) > t:
            yield TimeoutEvent(t)
        yield
