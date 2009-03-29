import os, os.path
import zipfile, os, os.path, sys
import logging
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename='simulator.log',
                    filemode='w')

import sr, motor, dio, vis, events

FPS = 20

#Got a zip file in sc.success, and robot.py is in the path
#Start the simulation

logging.debug("Starting simulation")
zippath = os.path.join(os.getcwd(), "face")
sys.path.insert(0, zippath)
from simthread import SimThread
simthread = SimThread(zippath, FPS)
simthread.start()
