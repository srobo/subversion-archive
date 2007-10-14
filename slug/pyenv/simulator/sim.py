from trampoline import Trampoline
import time
from physics import World

def sim_time():
    return World.time

time.time = sim_time

t = Trampoline()
p = World()
t.addtask(p.physics_poll())
t.schedule()
