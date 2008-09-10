#!/usr/bin/python
import pkg_resources
pkg_resources.require("TurboGears")
from turbogears import update_config, start_server, config

import cherrypy
cherrypy.lowercase_api = True
from os.path import *
import os, sys, logging

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

#Determine if we have ll_core
try:
    pkg_resources.require("ll_core")

    if config.get( "server.daemon", False ):
        from ll import daemon
        me = daemon.Daemon(
            stdout = config.get( "server.logfile", "/dev/null" ),
            stderr = config.get( "server.logfile", "/dev/null" ),
            pidfile = config.get( "server.pidfile", None ) )

        logging.raiseExceptions = False
        me.openstreams()
        me.start()

except pkg_resources.DistributionNotFound:
    pass

# Verify that the required config options are present:
req = ["ldap.anonuser", "ldap.anonpass", "user.header"]

for opt in req:
    if config.get( opt ) == None:
        print "Missing config option '%s', so quitting." % opt
        sys.exit(-1)

start_server(Root())
