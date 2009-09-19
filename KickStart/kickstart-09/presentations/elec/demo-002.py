# Demo: Wait for switch press before dropping ball
from sr import *

def main():
    while True:
        # Wait for switch press
        yield io.pin[0] == 1

        # Open gate
        pwm[0] = 0
        yield 3

        pwm[1] = 100
        yield 3


