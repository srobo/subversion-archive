import c2py

ADDRESS = 0x12

# Command
MOTOR_CMD_CONF = 0x01

# Directions
OFF = 0
FORWARD = 1
BACKWARD = 2
BRAKE = 3

def checkmotor():
    try:
        c2py.writeworddata( ADDRESS, MOTOR_CMD_CONF, 0, 0 )
    except c2py.I2CError:
        return False
    return True

def __set__( channel, speed ):
    
    speed = float(speed)

    if speed > 100:
        speed = 100
    elif speed < -100:
        speed = -100

    if channel not in [0,1]:
        raise ValueError, "Incorrect channel set"
    
    if speed > 0:
        dir = FORWARD
    else:
        dir = BACKWARD

    speed = abs(int(speed * 3.28))

    v = speed | (dir << 9) | (channel<<11)
    c2py.writeworddata( ADDRESS, MOTOR_CMD_CONF, v, 1 )

def setspeed(*args):
    if len(args) == 1:
        __set__( 0, args[0] )
        __set__( 1, args[0] )
    elif len(args) == 2:
        __set__( 0, args[0] )
        __set__( 1, args[1] )
    else:
        raise TypeError, "setspeed takes one or two numeric arguments"
