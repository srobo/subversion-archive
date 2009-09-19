# Demo: Drop balls periodically
from sr import *

def main():
    while True:
        # Open gate
        pwm[0] = 0
        yield 3

        pwm[1] = 100
        yield 3
