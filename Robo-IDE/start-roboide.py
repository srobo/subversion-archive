#!/usr/bin/python
import pkg_resources
pkg_resources.require("TurboGears")

from turbogears import update_config, start_server, config
import cherrypy
cherrypy.lowercase_api = True
from os.path import *
from os import getpid
import sys

# first look on the command line for a desired config file,
# if it's not on the command line, then
# look for setup.py in this directory. If it's not there, this script is
# probably installed
if len(sys.argv) > 1:
    update_config(configfile=sys.argv[1], 
        modulename="roboide.config")
elif exists(join(dirname(__file__), "setup.py")):
    update_config(configfile="dev.cfg",modulename="roboide.config")
else:
    update_config(configfile="prod.cfg",modulename="roboide.config")

from roboide.controllers import Root

# Write PID to file
pidfile = config.get( "server.pidfile" )
if pidfile != None:
    f = open( pidfile, "w" )
    f.write( str( getpid() ) + "\n" )
    f.close()

start_server(Root())
