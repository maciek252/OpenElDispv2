#!/bin/sh
/home/maciek/d/arduino-1.6.7/hardware/tools/avr/bin/avrdude -C/home/maciek/d/arduino-1.6.7/hardware/tools/avr/etc/avrdude.conf  -v -v -v -v -patmega328p -c arduino -P/dev/ttyUSB1 -b 57600 -Uflash:w:$1:i 
