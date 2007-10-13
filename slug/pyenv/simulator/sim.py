from trampoline import Trampoline
from physics import World

t = Trampoline()
p = World()
t.addtask(p.physics_poll())
t.schedule()
