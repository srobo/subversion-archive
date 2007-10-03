#These variable (with names in all upper case) are constants. They never
#change whilst the program is running. Unlike some other programming
#languages it is possible to change these. Just don't!
#Define the colour BLACK as no red, no green and no blue
BLACK = 0, 0, 0

#These are the directions the hero's spaceship can move in
LEFT = [-1, 0]
RIGHT = [1, 0]
STILL = [0, 0]

#This is the size (in pixels) of the game area.
SIZE = WIDTH, HEIGHT = 640, 480

#The higher the alienmissilechange, the less likely it is a missile is fired
#by the aliens (So higher number here makes it easier!)
ALIENMISSILECHANCE = 3000
#This is how far (in pixels) the aliens drop down for each time they cross
#the screen. The lower this number is, the slower they drop and the easier
#it is
ALIENDROPSPEED = 10
