#!/usr/bin/python
import sys, logging, os, os.path

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s %(message)s',
                    stream = sys.stdout)

os.putenv("LD_LIBRARY_PATH", "/usr/local/lib")

print "Initialising trampoline..."
try:
    loc = os.path.join(os.curdir, "robot.zip")
    sys.path.insert(0, loc)
    print "%s added to python path." % loc
    import robot
    print "User robot code import succeeded"
    import dio, motor, pwm, vis, c2py
    print "Peripheral libraries imported"
    import trampoline
    print "Trampoline imported"
    t = Trampoline()
    print "Trampoline initialised"
    print "Starting trampoline"
    t.schedule()
except:
    print "Could not load user code!"
    print "Error: "
    print sys.exc_info()
