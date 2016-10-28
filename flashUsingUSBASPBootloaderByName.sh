#!/bin/sh
/home/maciek/d/arduino-1.6.8/hardware/tools/avr/bin/avrdude -C/home/maciek/d/arduino-1.6.8/hardware/tools/avr/etc/avrdude.conf  -v -v -v -v -pm168p -cusbasp -Pusb -Uflash:w:$1:i 
