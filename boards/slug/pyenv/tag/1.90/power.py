#import c2py
from repeat import *

ADDRESS = 0x4f # 4F is the second generation address

# Command
CMD_IDENTIFY = 0x00
CMD_LED = 0x01
CMD_DIPSWITCH = 0x02
CMD_BUTTON = 0x03
CMD_SLUG_POWER = 0x04
CMD_SERVO_POWER = 0x05
CMD_MOTOR_POWER = 0x06
CMD_BATTERY = 0x07
CMD_VOLT = 0x08
CMD_AMP = 0x09
CMD_BEEGEES = 0x0a
CMD_TEST = 0x0b
CMD_BUTTON_FAKE = 0x0c
CMD_RTS = 0x0d
CMD_CTS = 0x0e
CMD_XBE = 0x0f

#functions commented out are correct but currently unusable due to firmware constraints, this will be addressed shortly
def setleds(led,val):
    temp = powerread(ADDRESS, CMD_LED)[0]
    temp = temp & ~(1<<led)
    temp |= val <<led
    while True:    
        powerwrite(ADDRESS,CMD_LED,[temp])
        if temp == powerread(ADDRESS, CMD_LED)[0]:
            break

def getleds(led):
    return (((powerread(ADDRESS, CMD_LED)[0])&(1<<led))>>led)

def getswitches(switch):
    return ((powerread(ADDRESS, CMD_DIPSWITCH)[0] & (1<<switch))>>switch)


def getbutton():
    return powerread(ADDRESS, CMD_BUTTON)[0]

def setbutton():
    while True:
        powerwrite(ADDRESS,CMD_BUTTON,[0])
        if 0 == powerread(ADDRESS, CMD_BUTTON)[0]:
            break

def slugpoweroff():
     while True:
        powerwrite(ADDRESS,CMD_SLUG_POWER,[0])
        # no escape from doom....

def setservopower(val):
    while True:
        powerwrite(ADDRESS,CMD_SERVO_POWER,[val])
        if powerread(ADDRESS,CMD_SERVO_POWER)[0]==val:
            break

def getservopower():
    return powerread(ADDRESS,CMD_SERVO_POWER)[0]



def setmotorpower(val):
    while True:
        powerwrite(ADDRESS,CMD_MOTOR_POWER,[val])
        if powerread(ADDRESS,CMD_MOTOR_POWER)[0]==val:
            break

def getmotorpower():
    return powerread(ADDRESS,CMD_MOTOR_POWER)[0]


def clearwatchdog():
    while True:
        powerwrite(ADDRESS,CMD_BEEGEES,[1])
        if powerread(ADDRESS,CMD_BEEGEES)[0]==1:
            break

def getxbects():
    return powerread(ADDRESS,CMD_CTS)[0]

def getxbrts():
    return powerread(ADDRESS,CMD_RTS)[0]

# def setxbrts(val):
#     while True:
#         powerwrite(ADDRESS,CMD_RTS,[val])
#         if powerread(ADDRESS,CMD_RTS)[0]==val:
#             break

# def getxb():
#     return powerread(ADDRESS,CMD_XBE)[0]

# def setxb(val):
#     while True:
#         powerwrite(ADDRESS,CMD_XBE,[val])
#         if powerread(ADDRESS,CMD_XBE)[0]==val:
#             break
