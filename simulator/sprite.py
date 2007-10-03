import pygame
from constants import *

class sprite:
    '''A sprite is a graphical thing. It has a stored image which is what you
    see and a position on the screen. Everything is a sprite!'''
    
    def __init__(self, filename):
        '''Load a graphic and create a rect which represents the space it
        uses up.'''
        #Load the image to use from the disk. The convert turns it into
        #the right format for use with pygame - better to do it now than every
        #time the image has to be drawn. Faster!!!
        self.image = pygame.image.load(filename).convert()
        #The rect(angle) represents the size of the image. A rectangle has
        #dimensions (width and height) as well as a position. 
        #This line gets a rectangle just big enough to contain the image and
        #stores it in self.rect
        self.rect = self.image.get_rect()
        
    def blit(self, surface, dirty):
        '''Write (blit) the image onto the surface'''
        #set_clip sets the bit of the surface you're working on.
        #This really speeds stuff up, as you don't have to redraw the
        #whole screen
        surface.set_clip(self.rect)
        #This copies the image onto the surface. Self.rect is used to position
        #it.
        surface.blit(self.image, self.rect)
        #This cleares the clipping on the surface - a good idea so that in 
        #future you don't have a weird clipping set that can be VERY confusing.
        surface.set_clip()
        #Dirty keeps track of what bits of the surface have changed (SPEED)
        dirty.append(self.rect)
        
    def wipe(self, surface, dirty):
        '''Paint over the sprite with black.'''
        #Changing a surface so track this with the dirty list
        dirty.append(self.rect)
        #Here you set a clipping region so that the fill doesn't flood the
        #whole screen with black.
        surface.set_clip(self.rect)
        surface.fill(BLACK)
        #Must always remember to clear the clipping!
        surface.set_clip()
    
    def move(self, x, y):
        '''Move the rectangle x accross and y down'''
        #self.rect.move is a funcion. It doesn't change self.rect - it creates
        #a new rect the same size as self.rect but moved by x and y. This new
        #rectangle is then stored in self.rect. This really caught me out...
        
        #The x and y are passed to rect.move in square brackets because
        #rect.move wants them as a *list* - no idea why.
        self.rect = self.rect.move([x, y])
    
    def moveandwipe(self, surface, dirty, x, y):
        '''This wipes the current space used by the sprite with black and then
        moves the rectangle. Useful for things which are moving along, not
        leaving trails behind them.'''
        sprite.wipe(self, surface, dirty)
        sprite.move(self, x, y)
        
    def hit(self, otherrect):
        '''This returns True if the rectangle passed to the function intersects
        with the sprite - kaboom!'''
        return self.rect.colliderect(otherrect)
