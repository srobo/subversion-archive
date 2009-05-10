#!/bin/sh

modprobe i2c-gpio
modprobe i2c-dev

exec /etc/init.d/rcS
