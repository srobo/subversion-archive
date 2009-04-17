# mainly sourced from http://trac.srobo.org/wiki/RobotAPI

# Constants
motor = power_switch = timeout = GOLF = SQUIRREL = RED = BLUE = GREEN = YELLOW = VISION_HEIGHT = VISION_WIDTH = 13

# Coroutinens

def add_coroutine(func):
	pass

def coroutine(func):
	pass

# Motor

# -100.0 < a,b < 100.0
def setpower(a, b=0):
	pass

# read the current motor power
def readpower(a):
	pass

# We shall be implementing this in future releases:
#def setspeed(a, b=0):
#	pass

# JointIO

# Read the digital value of pin 3:
def readpin(a):
	pass

# Read the analogue reading from pin 3
def readapin(a):
	pass

# Set output 1 high:
def setoutput(a,b):
	pass

class io():
	pin = [0,1,2,3,4,5,6,7]
	apin = [0,1,2,3,4,5,6,7]

# Event

class event():
	class io():
		pins = [0,1,2,3,4,5,6,7]
		apins = [0,1,2,3,4,5,6,7]

	class power_switch():
		switches = [0,1,2,3]
		vals = [0,1,2,3]

	class vision():
		class blob():
			x = y = mass = colour = height = width = 2
		blobs = []

# Logic ExpressionS

def Or(*args):
	pass

def And(*args):
	pass

# PWM

# set servo SERVO_NUMBER to position 0.0 <= POS <= 100.0
def setpos(n, pos):
	pass

# read servo position - returns servo position
def readpos(n):
	pass

def setlcd(n, msg=''):
	pass

lcd = [0,1,2,3]

# Power

def getled(a):
	pass

#similar to jio inputs...
def setled(a,b):
	pass

def setleds(a,b):
	pass

class power():
	led = [0,1,2,3,4,5,6,7]
	switch = [0,1,2,3]

# Vision

def vision():
	pass
