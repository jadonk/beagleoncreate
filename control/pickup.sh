#!/bin/bash

# From http://wh1t3s.com/2009/05/14/reading-beagleboard-gpio/

# Orginally /usr/bin/readgpio, Modified by Mark A. Yoder 20-Jul-2011

#
# Toggle a GPIO input


echo 9:off >/dev/servodrive0

echo 7:off >/dev/servodrive0


echo 9:100 > /dev/servodrive0

sleep 1

echo 9:off > /dev/servodrive0

echo 7:-88 > /dev/servodrive0

sleep 2



#Turn suction pump on

echo '1' > /dev/gpio141/value
sleep 5

#Turn suction pump off
echo '0' > /dev/gpio141/value



echo 7:100 > /dev/servodrive0

sleep 3

echo 9:-100 >/dev/servodrive0

sleep 1


echo 7:off > /dev/servodrive0

echo 9:off > /dev/servodrive0
