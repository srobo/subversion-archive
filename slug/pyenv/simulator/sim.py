import pygame
import gui
import threading
import sys
import time
import Queue
import time
from http import checkout
import zipfile, os.path
import getsrc

import motor, dio, vis, events

FPS = 10

#1. Try to get some code!
sc = getsrc.SourceLoader()
sc.main()
if sc.success == False:
    sys.exit(0)

#Got a zip file in sc.success, and robot.py is in the path
zippath = sc.success
del sc
z = zipfile.ZipFile(zippath)
#Read in the files in the zip file and their contents
files = [(os.path.join(zippath, a), z.read(a)) for a in z.namelist()]

#Can now bring in the trampiline which will import robot.py
from trampoline import Trampoline
from physics import World

class SimThread(threading.Thread):
    def __init__(self, drawqueue, watchpath, fps):
        super(SimThread, self).__init__()
        self.p = World(drawqueue, fps)
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

class SimDisplay(threading.Thread):
    BLACK = (0,0,0)

    def __init__(self, fps):
        super(SimDisplay, self).__init__()
        pygame.init()

        self.fps = fps
        
        self.screen = pygame.display.set_mode((640, 640))
        self.clk = pygame.time.Clock()
        self.simdrawqueue = Queue.Queue()

    def run(self):
        dirty = []
        lastdirty = []

        while True:

            dirty = lastdirty[:] #Make a copy of lastdirty
            lastdirty = []

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    return

            try:
                #Get the dirty for the set after the one just drawn
                pos = self.simdrawqueue.get_nowait()

                #Get the last in the queue - sim going too fast
                while not self.simdrawqueue.empty():
                    pos = simdrawqueue.get_nowait()

                self.screen.fill(SimDisplay.BLACK)
                for poly in pos:
                    poly.blit(self.screen)
                    lastdirty.append(poly.get_rect())

            except:
                #Not going fast enough
                pass

            pygame.display.update(dirty)
            self.clk.tick(self.fps)

display = SimDisplay(FPS)
display.start()

simthread = SimThread(display.simdrawqueue, zippath, FPS)
simthread.start()

gtkthread = gui.SimGUI(simthread.breakpoints, simthread.breaklock,
        simthread.fromsimq, simthread.tosimq, files)
gtkthread.run()
