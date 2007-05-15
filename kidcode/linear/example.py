from robot import motor, digitalio
from vision import blobfinder
from time import time, sleep
from math import tan


left_motor = motor(1)
right_motor = motor(2)
blobs = blobfinder()

state = "IDLE"

while True:
    print "State is: %s" % state
    if state == "IDLE":
        #Stop where we are whilst thinking
        left_motor.set_speed(0)
        right_motor.set_speed(0)
        #Check for blobs
        cansee = blobs.get_blobs()
        if cansee:
            #Go through list of tokens
            for token in cansee:
                #if the token is red, go to it
                if token[2] == 0:
                    #Red token, adjust heading
                    #Very rough & ready measurement!!!
                    #Presume distance is proportional
                    #to distance up image..... Yuch.
                    distance = (240-token[1]) / 30
                    #30 = number pulled from air

                    heading = tan((token[0] - 160)/
                                  distance)

                    print "Going for token at distance %f and heading %f" % (distance, heading)
                    
                    if heading > 0:
                        #Turn right
                        left_motor.set_speed(-5)
                        right_motor.set_speed(5)
                    else:
                        left_motor.set_speed(5)
                        right_motor.set_speed(-5)

                    starttime = time()
                    state = "SETTING_HEADING"
        else:
            #Can't see anything, move a bit then try again
            state = "MOVEABIT"
    elif state == "MOVEABIT":
        #Turn the motors on for a bit, then wait until moved
        left_motor.set_speed = 5
        right_motor.set_speed = 5
        #Store current time
        starttime = time()
        state = "WAITTOMOVE"
    elif state == "WAITTOMOVE":
        #Wait until current time = starttime + 5s
        if time() - starttime > 5:
            #Stop and go to idle state
            left_motor.set_speed = 0
            right_motor.set_speed = 0
            state = "IDLE"
    elif state == "SETTING_HEADING":
        #Presume motor turn time is proportional to heading needed
        #This would probably be done with proper position feedback
        if time() - starttime > abs(heading):
            left_motor.set_speed(5)
            right_motor.set_speed(5)
            starttime = time()
            state = "GET_TOKEN"
    elif state == "GET_TOKEN":
        #Go forward until reached the token
        #Done on time. Really sucks
        #Remember, distance was set in IDLE
        #Naively assume distance is proportional to
        #travel time..... Yeah, right...
        if time() - starttime > distance:
            left_motor.set_speed = 0
            right_motor.set_speed = 0
            state = "PICK_UP_TOKEN"
    else:
        #Can't be arsed with more states
        state == "IDLE"

    #Slow everything down!
    sleep(1)

