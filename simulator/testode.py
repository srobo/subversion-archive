import pygame
import ode
import time
from math import sqrt
from poly import poly
import random

pygame.init()

BLACK = 0, 0, 0
WHITE = 255, 255, 255

METRE = 640/8

POWER = 4 #Watts

screen = pygame.display.set_mode((640, 640))
clk = pygame.time.Clock()

def scalp(vec, scal):
    vec[0] *= scal
    vec[1] *= scal
    vec[2] *= scal

def length (vec):
    return sqrt (vec[0]**2 + vec[1]**2 + vec[2]**2)

world = ode.World()
world.setGravity( (0, 0, -9.81) )
#world.setERP(0.8) #Error correction per time step
world.setCFM(1E-5) #Global constraint force mixing value

space = ode.Space() 

floor = ode.GeomPlane(space, (0, 0, 1), 0) #Plane perpendicular to the normal
wall1 = ode.GeomPlane(space, (1, 0, 0), 0)
wall2 = ode.GeomPlane(space, (-1, 0, 0), -8)
wall3 = ode.GeomPlane(space, (0, 1, 0), 0)
wall4 = ode.GeomPlane(space, (0, -1, 0), -8)

bodies = []

contactgroup = ode.JointGroup()

fps = 100
dt = 1.0/fps
lasttime = time.time()

robot = ode.Body(world)

M = ode.Mass()
M.setBox(80, 0.5, 0.5, 0.5) #Density, lx, ly, lz
robot.setMass(M)

robot.shape = "box"
robot.boxsize = (0.5, 0.5, 0.5)

geom = ode.GeomBox(space, lengths=robot.boxsize)
geom.setBody(robot)

robot.setPosition((1, 2, 0.25))

bodies.append(robot)

tokens = []

def maketoken():
    global world, space

    token = ode.Body(world)
    M = ode.Mass()
    M.setBox(30, 0.044, 0.044, 0.044)
    token.serMass(M)
    token.shape = "box"
    token.boxsize = (0.044, 0.044, 0.044)

    geom = ode.GeomBox(space, lengths=token.boxsize)
    return token

for i in range(30):
    x = random.random() * 8
    y = random.random() * 8
    token = maketoken()
    token.setPosition((x, y, 0.05))
    tokens.append(token)

def near_callback(args, geom1, geom2):
    contacts = ode.collide(geom1, geom2)

    world, contactgroup = args #Passed in through a tuple - can probably pass
        #anything in

    if geom1.__class__ == ode.GeomPlane and geom2.__class__ == ode.GeomPlane:
        return

    for c in contacts:
        c.setBounce(0.01)
        c.setMu(35)
        j = ode.ContactJoint(world, contactgroup, c)
        j.attach(geom1.getBody(), geom2.getBody())

def rotate3(m, v):
	"""Returns the rotation of 3-vector v by 3x3 (row major) matrix m."""
	return (v[0] * m[0] + v[1] * m[1] + v[2] * m[2],
		v[0] * m[3] + v[1] * m[4] + v[2] * m[5],
		v[0] * m[6] + v[1] * m[7] + v[2] * m[8])

force = 80

m1 = force
m2 = force

print pygame.joystick.get_count()

stick = pygame.joystick.Joystick(0)
stick.init()

SCALE = 100

while True:
    for event in pygame.event.get():
        if event.type == pygame.JOYAXISMOTION:
            if event.axis == 1:
                force = SCALE * event.value * -1
            elif event.axis == 0:
                balance = -event.value

    m1 = force * (balance + 1)/2
    m2 = force * (1-((balance + 1) / 2))

    screen.fill(BLACK)

    space.collide((world, contactgroup), near_callback)

    p0 = [x*METRE for x in robot.getRelPointPos((-0.25, -0.25, -0.25))[:2]]
    p1 = [x*METRE for x in robot.getRelPointPos((0.25, -0.25, -0.25))[:2]]
    p2 = [x*METRE for x in robot.getRelPointPos((0.20, 0.25, -0.25))[:2]]
    p3 = [x*METRE for x in robot.getRelPointPos((-0.20, 0.25, -0.25))[:2]]

    p = poly([p0, p1, p2, p3], (0,0), 0)
    p.blit(screen, [])
    del p

    for token in tokens:
        p0 = [x*METRE for x in token.getRelPointPos((-0.022, -0.022, -0.022))[:2]]
        p1 = [x*METRE for x in token.getRelPointPos((0.022, -0.022, -0.022))[:2]]
        p2 = [x*METRE for x in token.getRelPointPos((0.20, 0.022, -0.022))[:2]]
        p3 = [x*METRE for x in token.getRelPointPos((-0.20, 0.022, -0.022))[:2]]
        p = poly([p0, p1, p2, p3], (0,0), 0)
        p.blit(screen, [])
        del p

    pygame.display.flip()

    robot.addRelForceAtRelPos((0, m1, 0), (-0.2, 0, 0))
    robot.addRelForceAtRelPos((0, m2, 0), (+0.2, 0, 0))

    world.step(dt)

    contactgroup.empty()
    clk.tick(fps)
