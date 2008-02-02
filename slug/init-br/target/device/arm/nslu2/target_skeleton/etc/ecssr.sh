#!/bin/sh

modprobe i2c-ixp4xx
modprobe i2c-dev

#webcam module
modprobe gspca

exec /etc/init.d/rcS
