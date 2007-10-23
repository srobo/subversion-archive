import physics

OFF = 0
FORWARD = 1
BACKWARD = 2
BRAKE = 3

def __set__( channel, speed ):
    
    speed = float(speed)

    if speed > 100:
        speed = 100
    elif speed < -100:
        speed = -100

    if channel not in [0,1]:
        raise ValueError, "Incorrect channel set"
    
    if channel == 0:
        physics.World.motorleft = speed
    else:
        physics.World.motorright = speed

def setspeed(*args):
    if len(args) == 1:
        __set__( 0, args[0] )
        __set__( 1, args[0] )
    elif len(args) == 2:
        __set__( 0, args[0] )
        __set__( 1, args[1] )
    else:
        raise TypeError, "setspeed takes one or two numeric arguments"
