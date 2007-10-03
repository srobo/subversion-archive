import math
import pygame
from sprite import sprite

class Rover(sprite):
    def __init__(self, bottom, width):
        '''This prepares the ship for use.'''
        #First off we call the sprite init function to do most of the work.
        sprite.__init__(self, "robot.png")
        
        #Then set the initial position of the alien.
        self.fy = bottom - self.rect.height
        self.fx = width/2 - self.rect.width
        
        self.direction = float(0) 

        self.__setpos__(self.fx, self.fy)

        self.speed = 0

    def __setpos__(self, x, y):
        self.rect.left = int(x)
        self.rect.top = int(y)

    def getmovement(self):
        tmprect = self.rect.move(0,0)
        self.tmprect.left = self.fx + self.speed*math.sin(self.direction)
        self.tmprect.top = self.fy + self.speed*math.cos(self.direction)
   
    def move(self):

        self.speed*math.sin(self.direction),
                  self.speed*math.cos(self.direction))

    def moveandwipe(self, surface, dirty):
        '''This moves the ship and wipes behind it.'''
        
        #Same overload as with the move function
        if self.rect.left > 0 and self.movement == LEFT:
            sprite.moveandwipe(self, surface, dirty, LEFT[0], LEFT[1])
        if self.rect.right < self.width and self.movement == RIGHT:
            sprite.moveandwipe(self, surface, dirty, RIGHT[0], RIGHT[1])
        
