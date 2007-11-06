import pygame
import ode
import time
from math import sqrt, floor, ceil, asin, pi, acos
from poly import poly
import random
import sys
import threading

BLACK = 0, 0, 0
WHITE = 255, 255, 255
WIDTH = 8
METRE = 640/WIDTH
SCALE = 25
WHEELFORCE = 50

class World:
    motorleft = 0
    motorright = 0
    bumpers = {}
    blobs = []
    time = 0

    class Box:
        def __init__(self, density, width, x, y, z, world, space, geoms = None,
                ident = None):
            self.box = ode.Body(world)
            self.box.container = self
            self.width = width

            M = ode.Mass()
            M.setBox(density, width, width, width) #Density, lx, ly, lz
            self.box.setMass(M)

            self.box.shape = "box"
            self.box.boxsize = (width, width, width)
            
            if geoms == None:
                geoms = [ode.GeomBox(space, lengths=self.box.boxsize)]
                if ident != None:
                    geoms[0].ident = ident

            for geom in geoms:
                geom.setBody(self.box)

            self.geoms = geoms

            self.box.setPosition((x, y, z))

            self.rect = pygame.Rect(0,0,0,0)

        def getpoly(self):
            hw = self.width/2
            p0 = [x*METRE for x in self.box.getRelPointPos((-hw, -hw, -hw))[:2]]
            p1 = [x*METRE for x in self.box.getRelPointPos((hw, -hw, -hw))[:2]]
            p2 = [x*METRE for x in self.box.getRelPointPos((hw, hw, -hw))[:2]]
            p3 = [x*METRE for x in self.box.getRelPointPos((-(hw), hw, -hw))[:2]]
            
            p = poly([p0, p1, p2, p3], (0,0), WHITE, 0)
            self.rect = p.get_rect()
            
            if self.__class__ == World.Robot:
                if self.box.getRotation()[8] > 0:
                    p0 = [x*METRE for x in self.box.getRelPointPos((-0.05, 0.25, 0))[:2]]
                    p1 = [x*METRE for x in self.box.getRelPointPos((0.05, 0.25, 0))[:2]]
                    p2 = [x*METRE for x in self.box.getRelPointPos((0.05, 0.15, 0))[:2]]
                    p3 = [x*METRE for x in self.box.getRelPointPos((-0.05, 0.15, 0))[:2]]
                    c = poly([p0, p1, p2, p3], (0,0), BLACK, 0)
                    return [p, c]

            return [p]

        def getdirty(self):
            return self.rect

        def addRelForceAtRelPos(self, force, position):
            self.box.addRelForceAtRelPos(force, position)

        def getRelPointPos(self, pos):
            return self.box.getRelPointPos(pos)
        
        def getRotation(self):
            return self.box.getRotation()

        def getPosition(self):
            return self.box.getPosition()

    class Robot(Box):
        def __init__(self, world, space):

            def genplane(space, size, pos, transparant = False, ident = ""):
                plane = ode.GeomBox(None, size)
                plane.setPosition(pos)
                planet = ode.GeomTransform(space)
                planet.setGeom(plane)
                
                planet.trans = transparant
                planet.ident = ident
                return planet
            
            def genwheel(world, space, box, position):
                WHEELRAD = 0.04
                WHEELLENGTH = 0.01
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
            topplane = genplane(space, (0.5, 0.2, 0.01), (0, 0.15, -0.240),
                    ident="topplane")
            geoms.append(topplane)

            botplane = genplane(space, (0.5, 0.2, 0.01), (0, -0.15, -0.240),
                    ident="botplane")
            geoms.append(botplane)

            bumpl = genplane(space, (0.1, 0.1, 0.01), (-0.245, 0.255, -0.240),
                    True, "bumpl")
            geoms.append(bumpl)

            bumpr = genplane(space, (0.1, 0.1, 0.01), (0.245, 0.255, -0.240),
                    True, "bumpr")
            geoms.append(bumpr)

            bumpeat = genplane(space, (0.2, 0.1, 0.01), (0, 0.255, -0.240),
                    True, "bumpeat")
            geoms.append(bumpeat)

            World.Box.__init__(self, 500, 0.5, 1, 2, 0.255, world, space, geoms)
            
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
            self.lwj.setParam(ode.ParamVel, float(l)/100 * SCALE)
            self.rwj.setParam(ode.ParamVel, float(r)/100 * SCALE)

    class Token(Box):
        def __init__(self, world, space, x, y):
            World.Box.__init__(self, 470, 0.044, x, y, 0.05, world, space,
                    ident="token")

    def createtokens(self, world, space, number):
        tokens = []

        for i in range(number):
            x = random.random() * WIDTH
            y = random.random() * WIDTH
            token = self.Token(world, space, x, y)
            tokens.append(token)

        return tokens

    def __init__(self, draw, fps):

        self.draw = draw
        self.fps = fps

        self.world = ode.World()
        self.world.setGravity( (0, 0, -9.81) )
        #world.setERP(0.8) #Error correction per time step
        self.world.setCFM(1E-5) #Global constraint force mixing value
        #TODO: Figure out why this isn't -8

        self.space = ode.Space() 

        floor = ode.GeomPlane(self.space, (0, 0, 1), 0) #Plane perpendicular to the normal
        floor.ident = "floor"
        wall1 = ode.GeomPlane(self.space, (1, 0, 0), 0)
        wall1.ident = "wall1"
        wall2 = ode.GeomPlane(self.space, (-1, 0, 0), -WIDTH)
        wall2.ident = "wall2"
        wall3 = ode.GeomPlane(self.space, (0, 1, 0), 0)
        wall3.ident = "wall3"
        wall4 = ode.GeomPlane(self.space, (0, -1, 0), -WIDTH)
        wall4.ident = "wall4"

        self.bodies = []

        self.contactgroup = ode.JointGroup()

        self.robot = self.Robot(self.world, self.space)
        self.tokens = self.createtokens(self.world, self.space, 10)
    
    def near_callback(self, args, geom1, geom2):
        if geom1.__class__ == ode.GeomPlane and geom2.__class__ == ode.GeomPlane:
            return

        mu = 10

        try:
            if geom1.ident == "bumpl" or geom2.ident == "bumpl":
                if geom2.ident != "floor":
                    World.bumpers["bumpl"] = True
            if geom1.ident == "bumpr" or geom2.ident == "bumpr":
                if geom2.ident != "floor":
                    World.bumpers["bumpr"] = True
        except AttributeError:
            pass

        try:
            eaten = None
            if geom1.ident == "token" and geom2.ident == "bumpeat":
                eaten = geom1.getBody().container
            if geom2.ident == "token" and geom1.ident == "bumpeat":
                eaten = geom2.getBody().container
            
            if eaten != None:
                eaten.box.disable()
                for geom in eaten.geoms:
                    self.space.remove(geom)
                self.tokens.remove(eaten)
                return
        except AttributeError:
            pass

        try:
            if geom1.trans or geom2.trans:
                return
        except AttributeError:
            pass
        
        try:
            if (geom1.ident == "token" and geom2.ident == "floor") or \
              (geom2.ident == "token" and geom1.ident == "floor"):
                mu = 1
        except AttributeError:
            pass

        contacts = ode.collide(geom1, geom2)

        world, contactgroup = args #Passed in through a tuple - can probably pass
            #anything in

        for c in contacts:
            c.setBounce(0.001)
            c.setMu(mu)
            j = ode.ContactJoint(world, contactgroup, c)
            j.attach(geom1.getBody(), geom2.getBody())

    def physics_poll(self):
        clk = pygame.time.Clock()
                
        while True:
            yield None
            dt = 1.0/self.fps
            clk.tick(self.fps)

            self.robot.setspeed(World.motorleft, World.motorright)

            #Emulate camera
            campos = self.robot.getRelPointPos((0, 0.2, 0))
            camrot = self.robot.getRotation()[3:6]

            def subvec(a, b):
                return (b[0]-a[0],
                        b[1]-a[1],
                        b[2]-a[2])

            def cross(a, b):
                return (a[1] * b[2] - a[2] * b[1],
                        a[2] * b[0] - a[0] * b[2],
                        a[0] * b[1] - a[1] * b[0])

            def dot(a, b):
                return (a[0]*b[0]+
                        a[1]*b[1]+
                        a[2]*b[2])
            def mag(a):
                return sqrt(a[0]**2+a[1]**2+a[2]**2)
            
            World.blobs = []

            for token in self.tokens:
                pos = token.getPosition()
                relvec = subvec(campos, pos)
                relvec = (relvec[0] * -1,
                          relvec[1],
                          relvec[2])
                adotb = dot(relvec, camrot)
                
                cc = cross(relvec, camrot)
                
                angle = adotb / (mag(relvec) * mag(camrot))
                angle = (acos(angle) / pi) * 180

                if adotb > 0.2 and adotb < 6 and angle < 20:
                    mass =  (1/adotb) * 20
                    if cc[2] > 0:
                        World.blobs.append((angle/20, adotb/6, mass))
                    else:
                        World.blobs.append((angle/20, adotb/6, mass))

            World.bumpers = {}
            self.space.collide((self.world, self.contactgroup), self.near_callback)

            self.world.step(dt)
            World.time = World.time + dt

            todraw = []
            todraw.extend(self.robot.getpoly())

            for token in self.tokens:
                todraw.extend(token.getpoly())

            #As the queue size is 1 this blocks on the drawing thread
            self.draw.draw(todraw)

            self.contactgroup.empty()

if __name__ == "__main__":
    w = World()
    gen = w.physics_poll()

    while True:
        gen.next()
        w.clk.tick(FPS)
