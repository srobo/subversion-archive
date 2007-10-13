import pygame
import ode
import time
from math import sqrt, floor, ceil
from poly import poly
import random
import sys

BLACK = 0, 0, 0
WHITE = 255, 255, 255
WIDTH = 8
METRE = 640/WIDTH
POWER = 4 #Watts
SCALE = 100
FPS = 100

class World:
    class Box:
        def __init__(self, density, width, x, y, z, world, space):
            self.box = ode.Body(world)
            self.width = width

            M = ode.Mass()
            M.setBox(density, width, width, width) #Density, lx, ly, lz
            self.box.setMass(M)

            self.box.shape = "box"
            self.box.boxsize = (width, width, width)

            geom = ode.GeomBox(space, lengths=self.box.boxsize)
            geom.setBody(self.box)

            self.box.setPosition((x, y, z))

            self.rect = pygame.Rect(0,0,0,0)

        def blit(self, screen, dirty):
            hw = self.width/2
            p0 = [x*METRE for x in self.box.getRelPointPos((-hw, -hw, -hw))[:2]]
            p1 = [x*METRE for x in self.box.getRelPointPos((hw, -hw, -hw))[:2]]
            p2 = [x*METRE for x in self.box.getRelPointPos((hw, hw, -hw))[:2]]
            p3 = [x*METRE for x in self.box.getRelPointPos((-(hw), hw, -hw))[:2]]
            
            minx = min(p0[0], p1[0], p2[0], p3[0]) - 2
            maxx = max(p0[0], p1[0], p2[0], p3[0]) + 2
            miny = min(p0[1], p1[1], p2[1], p3[1]) - 2
            maxy = max(p0[1], p1[1], p2[1], p3[1]) + 2

            self.rect = pygame.Rect(floor(minx), floor(miny),
                            ceil(maxx-minx), ceil(maxy-miny))

            p = poly([p0, p1, p2, p3], (0,0), 0)
            p.blit(screen, dirty)
            del p

        def setdirty(self, dirty):
            dirty.append(self.rect)

        def addRelForceAtRelPos(self, force, position):
            self.box.addRelForceAtRelPos(force, position)

    class Robot(Box):
        def __init__(self, world, space):
            World.Box.__init__(self, 80, 0.5, 1, 2, 0.25, world, space)

    class Token(Box):
        def __init__(self, world, space, x, y):
            World.Box.__init__(self, 30, 0.044, x, y, 0.05, world, space)

    def createtokens(self, world, space, number):
        tokens = []

        for i in range(number):
            x = random.random() * WIDTH
            y = random.random() * WIDTH
            token = self.Token(world, space, x, y)
            tokens.append(token)

        return tokens

    def __init__(self):
        pygame.init()

        self.screen = pygame.display.set_mode((640, 640))
        self.clk = pygame.time.Clock()

        self.world = ode.World()
        self.world.setGravity( (0, 0, -9.81) )
        #world.setERP(0.8) #Error correction per time step
        self.world.setCFM(1E-5) #Global constraint force mixing value

        self.space = ode.Space() 

        floor = ode.GeomPlane(self.space, (0, 0, 1), 0) #Plane perpendicular to the normal
        wall1 = ode.GeomPlane(self.space, (1, 0, 0), 0)
        wall2 = ode.GeomPlane(self.space, (-1, 0, 0), -WIDTH)
        wall3 = ode.GeomPlane(self.space, (0, 1, 0), 0)
        wall4 = ode.GeomPlane(self.space, (0, -1, 0), -WIDTH)

        self.bodies = []

        self.contactgroup = ode.JointGroup()

        self.robot = self.Robot(self.world, self.space)
        self.tokens = self.createtokens(self.world, self.space, 30)

    
    def near_callback(self, args, geom1, geom2):
        if geom1.__class__ == ode.GeomPlane and geom2.__class__ == ode.GeomPlane:
            return

        contacts = ode.collide(geom1, geom2)

        world, contactgroup = args #Passed in through a tuple - can probably pass
            #anything in

        for c in contacts:
            c.setBounce(0.01)
            c.setMu(35)
            j = ode.ContactJoint(world, contactgroup, c)
            j.attach(geom1.getBody(), geom2.getBody())

    def physics_poll(self):
        dt = 1.0/FPS
        lasttime = time.time()

        stick = pygame.joystick.Joystick(0)
        stick.init()
        
        balance = 0

        while True:
            yield None
            dirty = []
            self.robot.setdirty(dirty)
            for token in self.tokens:
                token.setdirty(dirty)

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    sys.exit()
                if event.type == pygame.JOYAXISMOTION:
                    if event.axis == 1:
                        force = SCALE * event.value * -1
                    elif event.axis == 0:
                        balance = -event.value

            m1 = force * (balance + 1)/2
            m2 = force * (1-((balance + 1) / 2))

            self.space.collide((self.world, self.contactgroup), self.near_callback)

            self.robot.addRelForceAtRelPos((0, m1, 0), (-0.2, -0.2, 0))
            self.robot.addRelForceAtRelPos((0, m2, 0), (+0.2, -0.2, 0))

            self.world.step(dt)

            self.screen.fill(BLACK)

            self.robot.blit(self.screen, dirty)

            for token in self.tokens:
                token.blit(self.screen, dirty)

            pygame.display.update(dirty)

            self.contactgroup.empty()

if __name__ == "__main__":
    w = World()
    gen = w.step()

    while True:
        gen.next()
        w.clk.tick(FPS)
