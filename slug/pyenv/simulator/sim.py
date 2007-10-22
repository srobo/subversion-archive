import gui
import sys
import zipfile, os, os.path
import getsrc

import motor, dio, vis, events

FPS = 10

#1. Try to get some code!
sc = getsrc.SourceLoader()
sc.main()
if sc.success[0] == False:
    sys.exit(0)

#Got a zip file in sc.success, and robot.py is in the path
zippath, delzip = sc.success
del sc
z = zipfile.ZipFile(zippath)

#Read in the files in the zip file and their contents
files = [(os.path.join(zippath, a), z.read(a)) for a in z.namelist()]

from simthread import SimThread
from simdisplay import SimDisplay

display = SimDisplay(FPS)
display.start()

simthread = SimThread(display.simdrawqueue, zippath, FPS)
simthread.start()

gtk = gui.SimGUI(simthread.breakpoints, simthread.breaklock,
        simthread.fromsimq, simthread.tosimq, files)
gtk.run()

if delzip:
    try:
        os.remove(zippath)
    except:
        pass
sys.exit()
