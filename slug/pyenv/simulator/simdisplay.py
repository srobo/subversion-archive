import pygame, Queue, threading

class SimDisplay(object):
    BLACK = (0,0,0)

    def __init__(self):
        pygame.init()

        self.screen = pygame.display.set_mode((640, 640))
        self.clk = pygame.time.Clock()
        self.simdrawqueue = Queue.Queue()

        self.lastdirty = []

    def draw(self, polygons):
        dirty = self.lastdirty[:] #Make a copy of lastdirty
        self.lastdirty = []

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return

        self.screen.fill(SimDisplay.BLACK)
        for poly in polygons:
            poly.blit(self.screen)
            self.lastdirty.append(poly.get_rect())

        pygame.display.update(dirty)
