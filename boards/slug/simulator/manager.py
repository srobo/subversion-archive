#!/usr/bin/env python
import sys
import json
from select import select
import socket
import logging
import time
from multiprocessing import Pipe
import traceback
from debugger import Sim

logging.basicConfig(level=logging.DEBUG,
                    format = "%(asctime)s - %(levelname)s - %(message)s")


class SimulationAlreadyRunning(Exception):
    pass

class SocketClosed(Exception):
    pass

class Simulation:
    """
    Physics simulation
    """

    def __init__(self, team, project):
        logging.debug("Simulation created for team %d project %s" % (team, project))
        self.team = team
        self.project = project
        
        #TODO: Load code from SVN
        zip = open("robot.zip", "rb").read()
        self.pipe, otherend = Pipe()
        
        self.sim = Sim(zip, otherend)
        self.sim.start()

    def step(self, commands):
        """
        Step the simulation
        """
        #Send commands
        for command in commands:
            message, args = command[0], command[1:]
            self.pipe.send((message, args))

        #Receive data
        data = []
        while self.pipe.poll():
            data.append(self.pipe.recv())
        return data

    def end(self):
        self.pipe.send(("END", None))
        self.sim.join()


class Manager:
    """
    Manage a set of concurrent simulations.
    """
    def __init__(self, socket):
        #Key: Team number, Value: Simulation
        self.sims = {}
        self.socket = socket
        self.inbuffer = ""

    def start(self, team, project):
        """
        Start a simulation for a team
        """
        if team in self.sims:
            #Only one simulation per team
            logging.error("Simulation already running for team %d" % team)
            raise SimulationAlreadyRunning()

        self.sims[team] = Simulation(team, project)
    
    def run(self):
        try:
            while True:
                self.step()
        except SocketClosed, s:
            logging.debug("Connection to client dropped")
            #TODO: Stop simulations

    def step(self):
        #Stuff to send to client
        tosend = []

        #Commands in the form {team : [["command", arg1, ...], ...], ...}
        commands = self.readCommands()
        
        #Find commands for teams that don't have a simulator
        #and create simulators for them
        for team in commands:
            if not team in self.sims:
                logging.debug("Adding team %d" % team)
                self.start(team, commands[team][0])

                #Remove the creation message
                commands[team].pop(0)
        
        #Step each simulator
        for team, sim in self.sims.iteritems():
            output = sim.step(commands.get(team, []))
            if output:
                tosend.append((team, output))

        if tosend != []:
            e = json.JSONEncoder()
            try:
                data = e.encode(tosend)
            except:
                logging.error("Could not encode data")
                return

            try:
                self.socket.sendall(data)
            except:
                logging.error("Could not send data")
                raise SocketClosed()

    def readCommands(self):
        """
        Read commands from the socket.
        Return a {team : [command, ...], ...} dict
        """
        commands = {}
        d = json.JSONDecoder()
        while True:
            read, write, error = select([self.socket], [], [self.socket], 0)
            if error:
                raise SocketClosed()

            if read == []: #Nothing to read
                return commands

            length_before = len(self.inbuffer)
            try:
                self.inbuffer += self.socket.recv(4096)
            except:
                raise SocketClosed()

            if len(self.inbuffer) == length_before:
                logging.debug("Error in socket")
                raise SocketClosed()

            if self.inbuffer:
                logging.debug("Got %d bytes of data" % len(self.inbuffer))

            data = None
            try:
                data, end = d.raw_decode(self.inbuffer)
            except ValueError:
                end = 0
            
            self.inbuffer = self.inbuffer[end:].strip()
            if self.inbuffer != "":
                logging.debug("Got the following left in the inbuffer:")
                logging.debug(self.inbuffer)

            logging.debug(data)
            if isinstance(data, list):
                for command in data:
                    commands.setdefault(command["team"], []).append(command["command"])

if __name__ == "__main__":
    #create an INET, STREAMing socket
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # allow rebinding to socket immediately after process killed (development sanity)
    serversocket.bind(("127.0.0.1", 10006))
    serversocket.listen(5)

    while True:
        clientsocket, address = serversocket.accept()
        print "Connected to client"
        m = Manager(clientsocket)
        m.run()
