#!/bin/sh
/home/maciek/d/arduino-1.6.7/hardware/tools/avr/bin/avrdude -C/home/maciek/d/arduino-1.6.7/hardware/tools/avr/etc/avrdude.conf  -v -v -v -v -patmega328p -c arduino -P/dev/ttyUSB1 -b 115200 -Uflash:w:/tmp/build52c78b98861ed83f6d5ca5337523e39f.tmp/Drone_ELT_mod__168.ino.hex:i 
