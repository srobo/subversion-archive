import zipfile, StringIO, imp, sys, os.path, os
from multiprocessing import Process, Pipe
import sys
import logging

logging.basicConfig(level=logging.DEBUG,
                    format = "%(asctime)s - %(levelname)s - %(message)s")

from physics import World
from simserve import SimServe
import pyenv.addhack
import time

class FinishSimulation(Exception):
    pass

class Sim(Process):
    def __init__(self, zipfile, pipe):
        Process.__init__(self)
        self.pipe = pipe
        self.zipfile = zipfile
        logging.debug("Created a sim")

    def run(self):
        """
        Now running in a different process!
        """
        logging.debug("Running a sim in process %d" % os.getpid())
        self.physics = World()
        self.prepareRobot()
        logging.debug("Starting trampoline")
        #Can now bring in the trampiline which will import robot.py
        from pyenv.trampoline import Trampoline
        self.t = Trampoline()
        logging.debug("Adding polls")
        pyenv.addhack.add_coroutine(self.physics.physics_poll)
        pyenv.addhack.add_coroutine(self.sendPositions)
        pyenv.addhack.add_coroutine(self.processCommands)
        pyenv.addhack.add_coroutine(self.speedLimiter)
        import robot
        pyenv.addhack.add_coroutine(robot.main, game=0, colour=0)

        self.debugmode = False
        self.watchpath = ""

        self.breakpoints = []
        sys.settrace(self.trace)

        try:
           self.t.schedule()
        except FinishSimulation, ex:
            pass

        logging.debug("Simulation process exiting.")

    def speedLimiter(self):
        t = time.time()
        while True:
            d = 0.2 - (time.time() - t)
            t = time.time()
            if d > 0:
                logging.debug("Sleeping %f" % d)
                time.sleep(d)
            yield

    def prepareRobot(self):
        """
        Uncompress the contents of self.zipfile into an in-memory dictionary,
        and create import hooks that are capable of finding that code.
        """
        f = StringIO.StringIO()
        f.write(self.zipfile)

        zip = zipfile.ZipFile(f)
            
        #modules of the form {"robot" : "from sr import *...", ...}
        modules = dict([(os.path.splitext(z.filename)[0], zip.open(z.filename).read())
                            for z in zip. infolist() \
                            if os.path.splitext(z.filename)[1] == ".py"])

        class Loader:
            """
            An object capable of bringing the python in the contents string
            to life. This works as part of the import hooks structure.
            """
            def __init__(self, fullname, contents):
                self.fullname = fullname
                self.contents = contents

            def load_module(self, fullname):
                if fullname in sys.modules:
                    return sys.modules[fullname]

                mod = sys.modules.setdefault(fullname, imp.new_module(fullname))
                mod.__file__ = "<memory/%s>" % fullname
                mod.__loader__ = self

                code = compile(self.contents, mod.__file__, "exec")

                exec code in mod.__dict__
                return mod

        class Finder:
            """
            An object to provide loaders for modules present as strings in the
            modules dict.
            """
            def __init__(self, modules):
                self.modules = modules

            def find_module(self, fullname, path=None):
                if (fullname in self.modules) and (path == None):
                    return Loader(fullname, self.modules[fullname])

                return None

        #Register the finder with the system
        sys.meta_path.append(Finder(modules))

    def sendPositions(self):
        """
        A generator added as a background task to the trampoline. It sends the positions
        from the physics engine out for each step.
        """
        while True:
            yield
            #TODO: Consider only updating the positions every N steps.
            self.pipe.send(self.physics.getPositions())

    def processCommands(self):
        """
        Process commands coming down the pipe.
        """
        while True:
            yield
            if self.pipe.poll():
                msg, argument = self.pipe.recv()
                if   msg == "STOP":
                    logging.debug("Stopping execution of robot code")
                    self.debugmode = True
                elif msg == "END":
                    logging.debug("Ending simulation")
                    raise FinishSimulation()
                elif msg == "BREAK":
                    logging.debug("Adding a breakpoint")
                    self.breakpoints.append(argument)
                else:
                    logging.debug("Unknown command")


    def tracerobot(self, frame, event, arg):
        """
        This is called for each event in the robot code.
        """
        if event != "line":
            return self.tracerobot

        curlineno = frame.f_lineno
        curfile = frame.f_code.co_filename
        
        #Check to see if stop on breakpoint
        if (curfile, curlineno-1) in self.breakpoints:
            self.debugmode = True

        if self.debugmode:
            ns = frame.f_globals.copy()
            for k, v in frame.f_locals.copy().iteritems():
                ns[k] = v
            
            #Only send data suitable for pickling
            #TODO: Make this more comprehensive
            for k in ns.keys():
                if not (isinstance(ns[k], type(1)) or \
                        isinstance(ns[k], type(1.5)) or \
                        isinstance(ns[k], type("String"))):
                    ns.pop(k)

            self.pipe.send((curfile, curlineno, ns))
            
            #Block on run or step from the GUI
            while True:
                msg, argument = self.pipe.recv()
                if msg == "RUN":
                    self.debugmode = False
                    break
                elif msg == "STEP":
                    break
        else:
            self.pipe.send((curfile, curlineno, {}))

    def trace(self, frame, event, arg):
        """
        A trace routine to get tracerobot to be called before each line of
        robot code is executed.
        """
        if event == "call":
            if frame.f_code.co_filename.startswith("<memory/"):
                return self.tracerobot
            else:
                return None
        return trace

if __name__ == "__main__":
    pipe, otherend = Pipe()
    sim = Sim(open("robot.zip", "rb").read(), otherend)
    sim.start()
    i = 0
    while True:
        print pipe.recv()
        print i
        i += 1
        if i == 50:
            pipe.send(("END", None))
            sim.join()
            break
