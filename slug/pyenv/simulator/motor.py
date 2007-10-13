import c2py

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
        print "Wrongness.... should throw an error here"

    v = speed | (dir << 9) | (channel<<11)
    c2py.writeworddata( ADDRESS, MOTOR_CMD_CONF, v, 0 )

def setspeed( channel, speed ):
    dir = FORWARD
    if speed < 0:
        dir = BACKWARD
        speed = -speed

    if speed == 0:
        dir = OFF

    __set__( channel, dir, speed )

def brake( channel ):
    __set__( channel, BRAKE, 100 )


