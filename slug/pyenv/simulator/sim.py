import pygame
import gui
import threading
import sys
import time
import Queue
from trampoline import Trampoline
import time
from physics import World

pygame.init()

fps = 10

screen = pygame.display.set_mode((640, 640))
clk = pygame.time.Clock()

BLACK = (0,0,0)

def step():
    print "STEP"

simdrawqueue = Queue.Queue()
simmask = pygame.Rect(0, 0, 640, 640)

class SimThread(threading.Thread):
    def __init__(self, drawqueue, fps):
        threading.Thread.__init__(self)
        self.p = World(drawqueue, fps)
        self.t = Trampoline()
        self.t.addtask(self.p.physics_poll())
        self.curlineno = 0
        self.curfile = ""

        self.debugmode = False
        self.stepevent = threading.Event()
        self.stepevent.clear()

        self.breakpoints = {}
        self.breaklock = threading.RLock()
        self.localqueue = Queue.Queue()

    def getcurline(self):
        return self.curfile, self.curlineno

    def getdebugmode(self):
        return self.debugmode

    def setdebugmode(self, mode):
        self.debugmode = mode

    def tracerobot(self, frame, event, arg):
        if event == "line":
            self.curlineno = frame.f_lineno
            self.curfile = frame.f_code.co_filename
            self.breaklock.acquire()
            if (frame.f_code.co_filename, self.curlineno) in self.breakpoints:
                self.debugmode = True
            self.breaklock.release()

            if self.debugmode:
                ns = frame.f_globals.copy()
                for k, v in frame.f_locals.copy().iteritems():
                    ns[k] = v

                self.localqueue.put(ns)

                self.stepevent.wait()
                self.stepevent.clear()

        return self.tracerobot

    def trace(self, frame, event, arg):
        if event == "call":
            if "user" in frame.f_code.co_filename:
                return self.tracerobot
            else:
                return None
        return trace

    def run(self):
        sys.settrace(self.trace)
        self.t.schedule()

simthread = SimThread(simdrawqueue, fps)
simthread.start()

gtkthread = gui.gtkthread(simthread.getcurline, simthread.stepevent,
        simthread.breakpoints, simthread.breaklock, simthread.setdebugmode,
        simthread.getdebugmode, simthread.localqueue)
gtkthread.start()

dirty = []
lastdirty = []

while True:

    dirty = lastdirty[:] #Make a copy of lastdirty
    lastdirty = []

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()

    try:
        #Get the dirty for the set after the one just drawn
        pos = simdrawqueue.get_nowait()

        #Get the last in the queue - sim going too fast
        while not simdrawqueue.empty():
            pos = simdrawqueue.get_nowait()

        screen.fill(BLACK)
        for poly in pos:
            poly.blit(screen)
            lastdirty.append(poly.get_rect())

    except:
        #Not going fast enough
        pass

    pygame.display.update(dirty)
    clk.tick(fps)
