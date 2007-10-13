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
SCALE = 60
FPS = 100

class World:
    motorleft = 0
    motorright = 0

    class Box:
        def __init__(self, density, width, x, y, z, world, space, geoms = None):
            self.box = ode.Body(world)
            self.width = width

            M = ode.Mass()
            M.setBox(density, width, width, width) #Density, lx, ly, lz
            self.box.setMass(M)

            self.box.shape = "box"
            self.box.boxsize = (width, width, width)
            
            if geoms == None:
                geoms = [ode.GeomBox(space, lengths=self.box.boxsize)]

            for geom in geoms:
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

            topline = (p1[0]-p0[0], p1[1]-p0[1])
            p0[0] += topline[0]/4
            p0[1] += topline[1]/4

            p = poly([p0, p1, p2, p3], (0,0), 0)
            p.blit(screen, dirty)

            
            if self.__class__ == World.Robot:
                #print "LEFT: %.2f %.2f %.2f"  % self.lwb.getRotation()[3:6]
                #print "RIGHT: %.2f %.2f %.2f" %  self.rwb.getRotation()[3:6]
                if self.box.getRotation()[8] > 0:
                    c = [int(x*METRE) for x in self.box.getRelPointPos((0, 0, 0))[:2]]
                    pygame.draw.circle(screen, BLACK, c, 5, 0)

            del p

        def setdirty(self, dirty):
            dirty.append(self.rect)

        def addRelForceAtRelPos(self, force, position):
            self.box.addRelForceAtRelPos(force, position)

    class Robot(Box):
        def __init__(self, world, space):

            def genplane(space, size, pos):
                plane = ode.GeomBox(None, size)
                plane.setPosition(pos)
                planet = ode.GeomTransform(space)
                planet.setGeom(plane)
                return planet
            
            def genwheel(world, space, box, position):
                WHEELRAD = 0.04
                WHEELLENGTH = 0.01
                WHEELFORCE = 10
                body = ode.Body(world)

                M = ode.Mass()
                #Arguments:
                #Mass, Axis(1=x), Radius, Length
                M.setCappedCylinderTotal(0.03, 1, WHEELRAD, WHEELLENGTH)
                body.setMass(M)
                body.setPosition(position)

                geom = ode.GeomCapsule(space, radius=WHEELRAD, length=WHEELLENGTH)
                geom.setBody(body)

                joint = ode.HingeJoint(world)
                joint.attach(box, body)
                joint.setAnchor(position)
                #Rotates on X axis
                joint.setAxis( (1, 0, 0) )

                joint.setParam(ode.ParamVel, 0)
                joint.setParam(ode.ParamFMax, WHEELFORCE)

                return body, joint

            def gencastor(world, space, box, abspos):
                CASTORRADIUS = 0.008

                body = ode.Body(world)
                M = ode.Mass()
                M.setSphere(2000, CASTORRADIUS)
                body.setMass(M)
                body.setPosition(abspos)
            
                s = ode.GeomSphere(space, CASTORRADIUS)
                s.setBody(body)
            
                j = ode.BallJoint(world)
                j.attach(box, body)
                j.setAnchor(abspos)
            geoms = []

            #genplane arguments: space, size, position
            topplane = genplane(space, (0.2, 0.5, 0.01), (0, 0.15, -0.240))
            geoms.append(topplane)

            botplane = genplane(space, (0.2, 0.5, 0.01), (0, -0.15, -0.240))
            geoms.append(botplane)

            World.Box.__init__(self, 100, 0.5, 1, 2, 0.255, world, space, geoms)
            
            #genwheel arguments: world, space, container, absolute pos
            #returns a wheel body and a joint object
            self.lw, self.lwj = genwheel(world, space, self.box,
                    self.box.getRelPointPos((-0.245, 0, -0.215)))
            self.rw, self.rwj = genwheel(world, space, self.box,
                    self.box.getRelPointPos((+0.245, 0, -0.215)))

            #gencaster arguments: world, space, box, absolute pos
            #returns the castor body
            self.bf = gencastor(world, space, self.box,
                    (0, 0.2, -0.245))
            self.bb = gencastor(world, space, self.box,
                    (0, -0.2, -0.245)) 

        def setspeed(self, l, r):
            self.lwj.setParam(ode.ParamVel, l)
            self.rwj.setParam(ode.ParamVel, r)

    class Token(Box):
        def __init__(self, world, space, x, y):
            World.Box.__init__(self, 470, 0.044, x, y, 0.05, world, space)

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

        if geom1.__class__ == ode.GeomCapsule:
            #print geom1.__class__, geom2.__class__
            if geom2.getBody() != None:
                #print geom2.foo
                for c in contacts:
                    a = c.getContactGeomParams()[0]
                    print a
                    print geom2.getBody().getPosRelPoint(a)

        for c in contacts:
            c.setBounce(0.001)
            c.setMu(10)
            j = ode.ContactJoint(world, contactgroup, c)
            j.attach(geom1.getBody(), geom2.getBody())

    def physics_poll(self):
        dt = 1.0/FPS
        lasttime = time.time()

        while True:
            yield None
            dirty = []
            self.robot.setdirty(dirty)
            for token in self.tokens:
                token.setdirty(dirty)

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    sys.exit()

            self.robot.setspeed(World.motorleft, World.motorright)

            self.space.collide((self.world, self.contactgroup), self.near_callback)

            self.world.step(dt)

            self.screen.fill(BLACK)

            self.robot.blit(self.screen, dirty)

            for token in self.tokens:
                token.blit(self.screen, dirty)

            pygame.display.update(dirty)

            self.contactgroup.empty()

if __name__ == "__main__":
    w = World()
    gen = w.physics_poll()

    while True:
        gen.next()
        w.clk.tick(FPS)
