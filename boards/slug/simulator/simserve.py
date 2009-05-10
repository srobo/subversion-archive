import BaseHTTPServer
from time import sleep
from threading import Thread, Lock, Event
import SocketServer
import logging
from copy import copy

handlers = []
handlerslock = Lock()

class Handler(BaseHTTPServer.BaseHTTPRequestHandler):
    """
    A handler is spawned in a new thread. Add it to the list of handlers then
    wait for it's event property to be set before reading data set externally
    and returning data to the client.
    """
    def __init__(self, *args):
        self.line = None
        self.event = Event()
        self.polys = ""
        BaseHTTPServer.BaseHTTPRequestHandler.__init__(self, *args)

    def do_GET(self):
        global handlerslock, handlers
        
        #Send all polys or just update?
        self.all = False

        if "?" in self.path:
            args = self.path.split("?")[-1]
            if args == "all=1":
                self.all = True

        
        #Add this to the list of handlers blocked on a frame finishing
        handlerslock.acquire()
        handlers.append(self)
        handlerslock.release()
        
        #Wait for data
        self.event.wait()

        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        
        state = "RUNNING" #TODO: Get this from the debugger
        self.wfile.write('{"line":%d, "state":"%s", "polys":[%s]}\n' % \
                (self.line, state, self.polys))
        self.wfile.flush()
        self.wfile.close()

class ThreadingHTTPServer(SocketServer.ThreadingMixIn,
        BaseHTTPServer.HTTPServer):
    """
    Defining this multiply inherited class creates a threading HTTPServer.
    """
    pass

class SimServe:
    """
    Spawn & manage a HTTP Server to stream out graphics related to a
    simulation.
    """
    def __init__(self, fromsimq):
        """
        fromsimq receives current line & file from the debugger.
        """
        #Pump a new HTTPServer into a seperate thread and return
        self.httpd = ThreadingHTTPServer(('', 8000), Handler)
        self.server_thread = Thread(target=self.httpd.serve_forever)
        self.server_thread.setDaemon(True) #Exit on quit
        self.httpd.daemon_threads = True
        self.server_thread.start()

        self.fromsimq = fromsimq
        self.line = 0

    def updateCurrentLine(self):
        try:
            position = self.fromsimq.get(False)
            self.line = position[1]
        except:
            pass
    
    def draw(self, polys):
        self.updateCurrentLine()
        
        #Lock the handlers list, make a copy then clear it
        global handlerslock, handlers
        handlerslock.acquire()
        h = copy(handlers)
        handlers = []
        handlerslock.release() 

        #Copy in h

        def poly2path(p):
            """
            Turn the points from a poly object into a SVG path.
            """
            p = p.points
            if len(p) == 0:
                return

            s = "M %d %d " % (p[0][0], p[0][1])

            for i in range(1, len(p)):
                s += "L %d %d " % (p[i][0], p[i][1])
            return '"' + s.strip() + '"'

        paths = [poly2path(p) for p in polys]
        allpaths = ", ".join(paths)

        #Add the info to each handler and kick it into action
        for handler in h:
            try:
                handler.line = self.line
                handler.polys = allpaths
                handler.event.set()
            except:
                pass

        logging.debug("All handlers handled")
