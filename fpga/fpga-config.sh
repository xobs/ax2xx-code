#!/bin/sh
if [ -z $1 ]
then
	echo "Usage: $0 [fpga-file]"
	exit 1
fi

bitfile=$1

echo "Setting export of reset pin"
echo 135 > /sys/class/gpio/export 2> /dev/null
echo "setting reset pin to out"
echo out > /sys/class/gpio/gpio135/direction 2> /dev/null
echo "flipping reset"                       
echo 0 > /sys/class/gpio/gpio135/value 2> /dev/null
echo 1 > /sys/class/gpio/gpio135/value 2> /dev/null
                                      
echo "configuring FPGA"               
                                      
dd if=${bitfile} of=/dev/spidev2.0 bs=128
                                                             
echo "turning on clock to FPGA"                              
devmem2 0x020c8160 w 0x00000D2B
