import c2py

ADDRESS = 0x3f

# Command
CMD_GET_FIRMWARE = 0x00
CMD_WRITE_LEDS = 0x01
CMD_GET_BATTERY = 0x02
CMD_GET_CURRENT = 0x03
CMD_GET_SWITCHES = 0x04
CMD_SET_RAILS = 0x05
CMD_GET_RAILS = 0x06
CMD_SEND_CHAR = 0x07
CMD_GET_USB = 0x08
# 0x09 is reserved for alive packet

MAXERR = 10

def getbyte(cmd):
    count = 0
    while count < MAXERR:
        try:
            byte = c2py.readbytedata( ADDRESS, cmd, 1 )
            break
        except c2py.I2CError:
            count = count + 1
    
    if count == MAXERR:
        raise c2py.I2CError

    return byte

def setbyte(cmd, val):
    count = 0
    while count < MAXERR:
        try:
            c2py.writebytedata(ADDRESS, cmd, val, 1)
            break
        except c2py.I2CError:
            count = count + 1

    if count == MAXERR:
        raise c2py.I2CError

def getword(cmd):
    count = 0
    while count < MAXERR:
        try:
            val = c2py.readworddata(ADDRESS, cmd, 1)
            break
        except c2py.I2CError:
            count = count + 1

    if count == MAXERR:
        raise c2py.I2CError
    
    return val

def checkpower():
    try:
        getfirmware()
        return True
    except c2py.I2CError:
        return False

def getfirmware():
    return getword(CMD_GET_FIRMWARE)

def setleds(v):
    setbyte(CMD_WRITE_LEDS, v & 0x0F)

def getbattery():
    return getword(CMD_GET_BATTERY) & 0x3FF

def getcurrent():
    return getword(CMD_GET_CURRENT) & 0x3FF

def getswitches():
    return getbyte(CMD_GET_SWITCHES)

def setrails(val):
    setbyte(CMD_SET_RAILS, val)

def getrails():
    return getbyte(CMD_GET_RAILS)

def sendchar(char):
    setbyte(CMD_SEND_CHAR, char)

def getusbconnected():
    if getbyte(CMD_GET_USB) == 1:
        return True
    else:
        return False
