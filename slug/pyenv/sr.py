from motor import setspeed
from dio import *
from vis import *

vision = vispoll()
vision.next()
dio = diopoll()
dio.next()
