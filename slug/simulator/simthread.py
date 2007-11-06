import threading, Queue, sys

#Can now bring in the trampiline which will import robot.py
from trampoline import Trampoline
from physics import World
from simdisplay import SimDisplay

class SimThread(threading.Thread):
    def __init__(self, watchpath, fps):
        super(SimThread, self).__init__()
        self.display = SimDisplay()
        self.p = World(self.display, fps)
        self.t = Trampoline()
        self.t.addtask(self.p.physics_poll())

        self.debugmode = True
        self.watchpath = watchpath

        self.breakpoints = {}
        self.breaklock = threading.RLock()
        self.fromsimq = Queue.Queue()
        self.tosimq = Queue.Queue()

    def tracerobot(self, frame, event, arg):
        if event == "line":
            curlineno = frame.f_lineno
            curfile = frame.f_code.co_filename
            
            #Check to see if stop on breakpoint
            self.breaklock.acquire()
            if (curfile, curlineno-1) in self.breakpoints:
                self.debugmode = True
            self.breaklock.release()

            #Has the stop button been pressed?
            #TODO: Check nothing else can happen here
            try:
                msg = self.tosimq.get_nowait()
                if msg == "STOP":
                    self.debugmode = True
            except Queue.Empty:
                pass

            if self.debugmode:
                ns = frame.f_globals.copy()
                for k, v in frame.f_locals.copy().iteritems():
                    ns[k] = v

                self.fromsimq.put((curfile, curlineno, ns))
                
                #Block on run or step from the GUI
                while True:
                    msg = self.tosimq.get()
                    if msg == "RUN":
                        self.debugmode = False
                        break
                    elif msg == "STEP":
                        break
            else:
                self.fromsimq.put((curfile, curlineno, {}))

        return self.tracerobot

    def trace(self, frame, event, arg):
        if event == "call":
            if self.watchpath in frame.f_code.co_filename:
                return self.tracerobot
            else:
                return None
        return trace

    def run(self):
        sys.settrace(self.trace)
        self.t.schedule()
