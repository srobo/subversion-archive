from motor import setspeed
from dio import *
from vis import *
from pwm import setpos

vision = vispoll()
vision.next()
dio = diopoll()
dio.next()
