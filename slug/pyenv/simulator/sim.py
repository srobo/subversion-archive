import pygame
import gui
import threading
import sys
import time
import Queue
import time
from http import checkout
import zipfile, os.path


#1. Try to get some code!
import getsrc
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

pygame.init()

fps = 10

screen = pygame.display.set_mode((640, 640))
clk = pygame.time.Clock()

BLACK = (0,0,0)

simdrawqueue = Queue.Queue()

from trampoline import Trampoline
from physics import World

class SimThread(threading.Thread):
    def __init__(self, drawqueue, watchpath, fps):
        threading.Thread.__init__(self)
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

simthread = SimThread(simdrawqueue, zippath, fps)
simthread.start()

gtkthread = gui.SimGUI(simthread.breakpoints, simthread.breaklock,
        simthread.fromsimq, simthread.tosimq, files)
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
