import pygame, Queue, threading

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

