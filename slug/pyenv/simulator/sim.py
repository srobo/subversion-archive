import pygame
import gui
import threading
import sys
import time
from trampoline import Trampoline
import time
from physics import World

pygame.init()

screen = pygame.display.set_mode((840, 640))
clk = pygame.time.Clock()

curlineno = 0
curlinelock = threading.RLock()

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

simscreen = pygame.Surface((640,640))
simscreenlock = threading.RLock()
simdirty = []

class SimThread(threading.Thread):
    def __init__(self, screen, screenlock, dirty):
        threading.Thread.__init__(self)
        self.p = World(screen, screenlock, dirty)
        self.t = Trampoline()
        self.t.addtask(self.p.physics_poll())

    def run(self):
        sys.settrace(trace)
        self.t.schedule()

simthread = SimThread(simscreen, simscreenlock, simdirty)
simthread.start()

dirty = []

start = time.time()
wait = 0
pstop = 0

while True:

    a = time.time()
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        else:
            gui.process_event(event)

    a = time.time()
    curlinelock.acquire()
    wait += time.time() - a

    lineno = curlineno
    curlinelock.release()

    gui.showline(curlineno)
    gui.drawgui(screen, dirty)

    a = time.time()
    simscreenlock.acquire()
    wait += time.time() - a

    screen.blit(simscreen, (0, 0))
    dirty.extend(simdirty)

    while len(simdirty) > 0:
        simdirty.pop()

    simscreenlock.release()
    pstop += time.time() - a

    pygame.display.update(dirty)
    
    total = time.time() - start 
    print total, wait, wait/total, pstop
