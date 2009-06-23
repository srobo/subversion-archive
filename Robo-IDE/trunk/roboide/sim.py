from turbogears import controllers, expose, config
import sys
import traceback
import socket
import simplejson
from threading import Thread, Lock
import cherrypy
import logging

#logging.basicConfig(level=logging.DEBUG,
#                    format = "%(asctime)s - %(levelname)s - %(message)s",
#                    stream=sys.stdout)

log = logging.getLogger("roboide.controllers")

#key: team number
# value: {"state" : "STARTED" or "RECEIVING" or "PAUSED",
#         "file" : "robot.py",
#         "lineno" : 123
#         "data"   : {}
#         "current" : {} #From sim

sims = {}

class SocketController:
    def __init__(self):
        if config.get( "simulator.manager_host" ) == None or config.get( "simulator.manager_port" ) == None:
            raise "Simulator manager's port or host not configured"

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect(( config.get( "simulator.manager_host" ),
                              config.get( "simulator.manager_port" ) ))
        self.todraw = {}
        self.readThread = Thread(target=self.readSocket)
        self.readThread.start()
        print "SocketController instantiated"

    def sendall(self, s):
        self.socket.sendall(s)

    def readSocket(self):
        d = simplejson.decoder.JSONDecoder()
        while True:
            try:
                string = self.socket.recv(8096)
            except:
                print "Lost socket"
                traceback.print_exc()
                break

            data = None
            try:
                data, end = d.raw_decode(string)
            except ValueError:
                print "Error decoding"
                break

            string = string[end:]

            self.process(data)
    
    def process(self, data):
        """
        Process incoming data.
        """
        for team, output in data:
            if not team in sims:
                log.error("Received data for unknown team")
                continue
            
            if not isinstance(output, list):
                log.error("Team data not in the form of a list")
                continue

            if sims[team]["state"] == "STARTED":
                log.debug("Got first info for team %d" % team)
                sims[team]["state"] = "RECEIVING"

            if len(output) == 0:
                log.error("Zero length output for team %d" % team)
                continue

            #Output contains debug info: [u'<memory/robot>', 14, {}]
            #and position information
            posinfo = None
            for info in output:
                if isinstance(info, list):
                    #Debug info
                    sims[team]["file"] = info[0]
                    sims[team]["lineno"] = info[1]
                    sims[team]["data"] = info[2]
                elif isinstance(info, dict):
                    #Position  info
                    #Take the last one
                    posinfo = info

            if posinfo:
                sims[team]["current"] = posinfo

sc = SocketController()

class Sim(object):
#    @expose(template="simtest.templates.welcome")
#    def index(self):
#        import time
#        # log.debug("Happy TurboGears Controller Responding For Duty")
#        return dict(now=time.ctime())

    @expose("json")
    def getdata(self, teamno):
        try:
            teamno = int(teamno)
            #TODO: Check a sane team
        except:
            return {"result" : -1,
                    "error"  : "Invalid team no"}

        return sims[teamno]


    @expose("json")
    def start(self, teamno):
        try:
            teamno = int(teamno)
            #TODO: Check a sane team
        except:
            return {"result" : -1,
                    "error"  : "Invalid team no"}

        if teamno in sims:
            return {"result" : -1,
                    "error"  : "Sim already running"}

        sims[teamno] = {"state" : "STARTED",
                        "file"  : "",
                        "lineno" : 0,
                        "current" : {}}

        command = "[{\"team\" : 1, \"command\" : \"start\"}]"
        sc.sendall(command)
        return {"result" : 0}
