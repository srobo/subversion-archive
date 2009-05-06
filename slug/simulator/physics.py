import pymunk as pm
from pymunk import Vec2d
import ctypes
import math
import random
import time
X,Y = 0,1
### Physics collision types
COLLTYPE_DEFAULT = 0
COLLTYPE_MOUSE = 1

CAM_ANGLE=0.5
CAM_DISTANCE = 300

ROLLING_FRICTION = 2
ROBOT_MASS = 10
ROBOT_WIDTH = 20

BALL_MASS = 1
BALL_RADIUS = 2
BALL_COUNT = 20

ROBOT_TYPE = 100
BALL_TYPE = 101
WALL_TYPE = 102

WIDTH = 500
HEIGHT = 500

STEP = 1.0/60.0

VISION_WIDTH = 320
HALF_VISION_WIDTH = 160
VISION_HEIGHT = 240

BUMPER_SIZE = 1 #Offset from bumper position where a wall hit causes the bumper to trigger

motora = 100
motorb = 100
   
class World:
    def __init__(self):
        #Startup physics engine
        pm.init_pymunk()
        self.space = pm.Space()
        self.space.gravity = Vec2d(0.0, 0)

        self.setupBodyResetForces()
        self.addBalls()
        self.addRobot()
        self.addWalls()
        self.addBumpers()

    def setupBodyResetForces(self):
        #Don't have reset_forces() in pymunk
        self.cpBodyResetForces = pm._chipmunk.chipmunk_lib.cpBodyResetForces
        self.cpBodyResetForces.restype = None
        self.cpBodyResetForces.argtypes = [ctypes.POINTER(pm._chipmunk.cpBody)]
 
    def addBalls(self):
        ## Balls
        def randomBall():
            body = pm.Body(BALL_MASS, BALL_MASS)
            body.position = Vec2d(random.randint(100+BALL_RADIUS, 500-BALL_RADIUS-1),
                                random.randint(100+BALL_RADIUS, 500-BALL_RADIUS-1))
            shape = pm.Circle(body, BALL_RADIUS, Vec2d(0, 0))
            shape.collision_type = BALL_TYPE
            shape.friction = 0.5
            self.space.add(body, shape)
            return shape

        self.balls = [randomBall() for x in range(BALL_COUNT)]
        
    def addRobot(self):
        def create_poly(points, mass = 5.0, pos = (0,0)):
            moment = pm.moment_for_poly(mass,points, Vec2d(0,0))
            #moment = 1000
            body = pm.Body(mass, moment)
            body.position = Vec2d(pos)
            
            shape = pm.Poly(body, points, Vec2d(0,0))
            shape.friction = 1
            return body, shape

        def create_box(pos, size = 10, mass = 5.0):
            box_points = map(Vec2d, [(-size, -size), (-size, size), (size,size), (size, -size)])
            return create_poly(box_points, mass = mass, pos = pos)

        self.robotbody, self.robotshape = create_box(Vec2d(300,300), ROBOT_WIDTH, ROBOT_MASS)
        self.robotshape.collision_type = ROBOT_TYPE
        self.space.add(self.robotbody, self.robotshape)

        self.wheels = [Vec2d(-10, 6), Vec2d(10, 6)]

        #Enable callback for collisions between robot and balls
        self.space.add_collisionpair_func(ROBOT_TYPE, BALL_TYPE, self.robotHitBall)
        #Enable callback for collisions between robot and walls
        self.space.add_collisionpair_func(ROBOT_TYPE, WALL_TYPE, self.robotHitWall)

    def addWalls(self):
        def create_line(start, end):
            body = pm.Body(pm.inf, pm.inf)
            shape = pm.Segment(body, start, end, 0.0)
            shape.friction = 0.99
            shape.collision_type = WALL_TYPE
            self.space.add_static(shape)
            return shape

        ### Static line
        self.static_lines = [ create_line(Vec2d(0, 0), Vec2d(WIDTH, 0)),
                              create_line(Vec2d(WIDTH, 0), Vec2d(WIDTH, HEIGHT)),
                              create_line(Vec2d(WIDTH, HEIGHT), Vec2d(0, HEIGHT)),
                              create_line(Vec2d(0, HEIGHT), Vec2d(0, 0)) ]

    def addBumpers(self):
        self.bumpers = [{"x" : -20, "y" : 20, "hit" : False},
                        {"x" : 20, "y" : 20, "hit" : False}]

    def applyWheelforce(self, position, force):
        #Working in a coordinate system based on the body
        #1. Get the velocity in terms of the body
        bvelocity = self.robotbody.velocity.cpvrotate(self.robotbody.rotation_vector)
        
        #2. Find the tangential velocity of this wheel based on the rotation
        #Tangential Velocity = r x w Where w = angular speed * n
        #and Where n = unit vector in z direction
        tv = Vec2d(position.y * self.robotbody.angular_velocity * -1,
                   position.x * self.robotbody.angular_velocity)
        
        #Overall velocity
        v = bvelocity + tv
        #Velocity in the direction the motors are pushing
        vd = v.dot(Vec2d(0, 1)) * Vec2d(0, 1)
        #3. Friction term based on velocity
        friction = ROLLING_FRICTION * vd
        #4. Find a net force
        force = Vec2d(0, force)
        nforce = force - friction
        
        #5. Convert back to world coordinates to apply the force
        wforce = nforce.cpvrotate(self.robotbody.rotation_vector)
        #Docs say convert to world coordinates here - they mean rotate only!
        wpos = position.cpvrotate(self.robotbody.rotation_vector)
        self.robotbody.apply_force(wforce, wpos)


    def robotHitBall(self, shapeA, shapeB, contacts, normal_coef, data):
        """
        Called on a collision. Must not remove shapes or bodies from
        the space as Bad Things will happen.
        """
        for contact in contacts:
            position = self.robotbody.world_to_local(contact.position)
            #Is position on top axis?
            if position.y > ROBOT_WIDTH * 0.8:
                if isinstance(shapeA, pm.Circle):
                    self.toremove.append(shapeA)
                else:
                    self.toremove.append(shapeB)

        #Return true if they hit
        return True

    def robotHitWall(self, shapeA, shapeB, contacts, normal_coef, data):
        """
        Called on collision between the robot and a wall.
        """
        for contact in contacts:
            position = self.robotbody.world_to_local(contact.position)
            for bumper in self.bumpers:
                if abs(position.y - bumper["y"]) <= BUMPER_SIZE and \
                        abs(position.x - bumper["x"]) <= BUMPER_SIZE:
                    #Hit the bumper
                    bumper["hit"] = True
                else:
                    bumper["hit"] = False
        return True

    def getPositions(self):
        return {"robot" : {"x" : self.robotbody.position.x,
                           "y" : self.robotbody.position.y,
                           "angle" : self.robotbody.angle},
                "balls" : [{"x" : b.body.position.x,
                            "y" : b.body.position.y} for b in self.balls]}

    def getBlobs(self):
        blobs = [] #List of blobs in the robots view.
        for ball in self.balls:
            pos = ball.body.position
            bpos = self.robotbody.world_to_local(pos)
            
            dot = bpos.dot(Vec2d(0, 1)) #Distance
            angle = math.acos(dot/bpos.get_length()) #Angle
            if abs(angle) < CAM_ANGLE:
                blob = {"centrex" : HALF_VISION_WIDTH + \
                                angle / CAM_ANGLE * HALF_VISION_WIDTH,
                        "centrey" : dot/CAM_DISTANCE * VISION_HEIGHT,
                        "mass" : dot / VISION_HEIGHT * 100,
                        "colour" : 0}
                blobs.append(blob)

        return blobs

    def getBumpers(self):
        return self.bumpers

    def physics_poll(self):
        while True:
            self.cpBodyResetForces(self.robotbody._body)
            self.applyWheelforce(self.wheels[0], motora)
            self.applyWheelforce(self.wheels[1], motorb)

            ### Update physics
            self.toremove = []
            for bumper in self.bumpers:
                bumper["hit"] = False

            self.space.step(STEP)
            
            for ball in self.toremove:
                self.space.remove(ball)
                self.space.remove(ball.body)
                self.balls.remove(ball)

            yield 

if __name__ == "__main__":
    world = World()
    poll = world.physics_poll()
    while True:
        print poll.next()
        time.sleep(0.1)
