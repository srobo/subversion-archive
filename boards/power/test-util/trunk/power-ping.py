#!/usr/bin/env python
import time
import serial
import sys


port=sys.argv[1]#"/dev/ttyUSB0"
tty = serial.Serial(port, baudrate=57600, rtscts=0, xonxoff=0, timeout=5)

ping = 	'\x7E\x00\x0D\x80\x00\x01\x02\x03\x04\x05\x06\x07\x00\xff\x01\x02\x61'


while 1:
    
    raw_input()
    tty.write(ping)
    time.sleep(0.5)



while 1:
    
    for loc in ping:
        raw_input()
        print hex(ord(loc))
        tty.write(loc)
        time.sleep(0.5)


tty.close()



# 	0x7E, //  framing byte
# 	0, 0x0D, // length
# 	0x80,             // API identifier for ive recived a packet
# 	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, //# Source address -- this should be ignoored
# 	0, //rssi -ignoored
# 	0xff, // options -- ignoored
# 	0x01,0x02, // the actuall data!!!
# 	0x00};
