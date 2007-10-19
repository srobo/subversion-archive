import pygame
import gui
import threading
import sys
import time
import Queue
from trampoline import Trampoline
import time
from physics import World
import tgtk

pygame.init()

fps = 10

screen = pygame.display.set_mode((840, 640))
clk = pygame.time.Clock()

BLACK = (0,0,0)

def step():
    print "STEP"

#gui = gui.GUI(step)
guimask = pygame.Rect(640, 0, 200, 640)

simdrawqueue = Queue.Queue()
simmask = pygame.Rect(0, 0, 640, 640)

class SimThread(threading.Thread):
    def __init__(self, drawqueue, fps):
        threading.Thread.__init__(self)
        self.p = World(drawqueue, fps)
        self.t = Trampoline()
        self.t.addtask(self.p.physics_poll())
        self.curlineno = 0

    def getcurline(self):
        #TODO - Atomic... I think! Check
        return self.curlineno

    def tracerobot(self, frame, event, arg):
        if event == "line":
            self.curlineno = frame.f_lineno
        return self.tracerobot

    def trace(self, frame, event, arg):
        if event == "call":
            if "robot" in frame.f_code.co_filename:
                return self.tracerobot
            else:
                return None
        return trace

    def run(self):
        sys.settrace(self.trace)
        self.t.schedule()

simthread = SimThread(simdrawqueue, fps)
simthread.start()

gtkthread = tgtk.gtkthread()
gtkthread.start()

dirty = []
lastdirty = []

while True:

    dirty = lastdirty[:] #Make a copy of lastdirty
    lastdirty = []

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        else:
            pass
            #gui.process_event(event)

    #gui.showline(simthread.getcurline())
    #gui.drawgui(screen, dirty)

    try:
        #Get the dirty for the set after the one just drawn
        pos = simdrawqueue.get_nowait()

        #Get the last in the queue - sim going too fast
        while not simdrawqueue.empty():
            pos = simdrawqueue.get_nowait()

        screen.set_clip(simmask)

        screen.fill(BLACK)
        for poly in pos:
            poly.blit(screen)
            lastdirty.append(poly.get_rect())

        screen.set_clip(none)
    except:
        #Not going fast enough
        pass

    pygame.display.update(dirty)
    clk.tick(fps)
