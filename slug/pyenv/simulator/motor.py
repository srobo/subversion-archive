import physics

OFF = 0
FORWARD = 1
BACKWARD = 2
BRAKE = 3

def __set__( channel, dir, speed ):

    if speed > 100:
        speed = 100
    elif speed < 0:
        speed = 0

    if channel not in [0,1] or dir not in [OFF,FORWARD,BACKWARD,BRAKE]:
        print "Wrong channel or direction"
    
    if dir == OFF:
        speed = 0
    if dir == BACKWARD:
        speed = -speed
    if dir == BRAKE:
        speed = 0 #TODO: Do something special here

    if channel == 0:
        physics.World.motorleft = speed
    else:
        physics.World.motorright = speed

def setspeed( channel, dir, speed ):
    dir = FORWARD
    if speed < 0:
        dir = BACKWARD
        speed = -speed

    if speed == 0:
        dir = OFF

    __set__( channel, dir, speed )

def brake( channel ):
    __set__( channel, BRAKE, 100 )
