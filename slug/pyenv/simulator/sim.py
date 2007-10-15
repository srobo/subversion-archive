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

screen = pygame.display.set_mode((840, 640))
clk = pygame.time.Clock()

curlineno = 0
curlinelock = threading.RLock()

BLACK = (0,0,0)

def tracerobot(frame, event, arg):
    global curlineno, curlinelock
    if event == "line":
       curlinelock.acquire()
       curlineno = frame.f_lineno
       curlinelock.release()
    return tracerobot

def trace(frame, event, arg):
    if event == "call":
        if "robot" in frame.f_code.co_filename:
            return tracerobot
        else:
            return None
    return trace

def step():
    print "STEP"

gui = gui.GUI(step)
guimask = pygame.Rect(640, 0, 200, 640)

simdrawqueue = Queue.Queue()
simmask = pygame.Rect(0, 0, 640, 640)

class SimThread(threading.Thread):
    def __init__(self, drawqueue):
        threading.Thread.__init__(self)
        self.p = World(drawqueue)
        self.t = Trampoline()
        self.t.addtask(self.p.physics_poll())

    def run(self):
        sys.settrace(trace)
        self.t.schedule()

simthread = SimThread(simdrawqueue)
simthread.start()

dirty = []

while True:

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        else:
            gui.process_event(event)

    curlinelock.acquire()
    lineno = curlineno
    curlinelock.release()

    gui.showline(curlineno)
    gui.drawgui(screen, dirty)

    try:
        #Get the dirty for the set after the one just drawn
        pos = simdrawqueue.get_nowait()
        dirty.extend(pos[0])

        #Get the last in the queue
        while not simdrawqueue.empty():
            pos = simdrawqueue.get_nowait()

        screen.set_clip(simmask)

        screen.fill(BLACK)
        for poly in pos[1]:
            poly.blit(screen, dirty)

        screen.set_clip(none)
    except:
        pass

    pygame.display.update(dirty)
    clk.tick(25)
