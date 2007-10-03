#This big block brings in lots of things that have already done for you! Later
#you can have a look at how they all work, but for now you can just leave them.
import pygame,sys,random
from init import *
from interface import Rover
from constants import *

#This is a list, called dirty. We keep track of all the places on the screen
#that have changed - and we call these bits dirty. This makes it much quicker
#as the computer only has to redraw the bits which have changed.
dirty = []

rover = Rover(HEIGHT, WIDTH)

alien = alien(0, WIDTH/2)

while 1:
    #The clock makes sure that the game goes the same speed on all different
    #computers no matter how fast they are.
    clock.tick(300)
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        
    rover.moveandwipe(screen, dirty)
    
    #Finally the alien and the spaceship need to be printed onto the screen
    rover.blit(screen, dirty)
    
    pygame.display.update(dirty)
    dirty = []
