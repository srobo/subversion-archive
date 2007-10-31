import gui
import sys
import zipfile, os, os.path
import getsrc
import logging
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename='simulator.log',
                    filemode='w')

import sr, motor, dio, vis, events

FPS = 20

#1. Try to get some code!
sc = getsrc.SourceLoader()
logging.debug("Loading get source message box")
sc.main()
logging.debug("Get source result = " + str(sc.success))
if sc.success[0] == False:
    sys.exit(0)

#Got a zip file in sc.success, and robot.py is in the path
zippath, delzip = sc.success
del sc
logging.debug("Opening zipfile %s" % zippath)
z = zipfile.ZipFile(zippath)

#Read in the files in the zip file and their contents
files = [(os.path.join(zippath, a), z.read(a)) for a in z.namelist()]

#Start the simulation

from simthread import SimThread
logging.debug("Starting simulation")
simthread = SimThread(zippath, FPS)
simthread.start()
logging.debug("Starting GUI")
gtk = gui.SimGUI(simthread.breakpoints, simthread.breaklock,
        simthread.fromsimq, simthread.tosimq, files)
gtk.run()
logging.debug("GUI Finished. Delzip is " + str(delzip))
if delzip:
    try:
        os.remove(zippath)
    except:
        pass
sys.exit()
