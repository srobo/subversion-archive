import pygame
from constants import *

#These commands just get everything ready for our game
pygame.init()
screen = pygame.display.set_mode(SIZE)
#If you want the game to be full screen, un-comment the next line (by
#removing the # at the beginning)
#pygame.display.toggle_fullscreen()
clock = pygame.time.Clock()
