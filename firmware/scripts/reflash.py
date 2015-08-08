#!/usr/bin/python

"""
Reflash the wemo image using sam-ba bootloader
**requires 99-atmel_samba.rules in udev**
**requires samba_64**
John Donnal
"""

import serial
import subprocess
import time
import os

BOARD="at91sam4s4-wemo"
WEMO_DEVICE="/dev/serial/by-id/usb-MIT_WEMO_Control_Board-if00"
SAMBA_DEVICE="/dev/serial/by-type/sam-ba"
#SAMBA_DEVICE="/dev/ttyACM3"
HEX_FILE="../bin/flash.bin"
##CHANGE THIS TO YOUR SAMBA BINARY LOCATION##
SAMBA_EXEC="../../sam-ba_cdc_linux/sam-ba_64"
SAMBA_TCL="./scripts/samba_reflash.tcl"

print("##### WEMO Reflash Tool #####")
if(os.path.exists(WEMO_DEVICE)):

   s = serial.Serial(WEMO_DEVICE,115200)

   s.write("echo off\n")
   time.sleep(0.5)
   s.flushInput()
   s.write("config get serial_number\n")
   serial_no=s.readline().rstrip()
   print(" - detected Wemo Boad [%s]"%serial_no)
   
   print(" - entering bootloader")
   s.write("restart bootloader\n")
   time.sleep(1)
   s.close()
   time.sleep(5) #wait for device to re-enumerate
elif(os.path.exists(SAMBA_DEVICE)):
   print(" - device already in bootloader")
else:
   print(" ERROR: cannot detect device")
   exit(1)

if(not os.path.exists(SAMBA_DEVICE)):
   print(" ERROR: failed to enter bootloader")
   exit(1)

SAMBA_RAW_DEVICE=raw_input("enter ACM device: /dev/ttyACMX\n> ")
print(" - reflashing chip")
#sam-ba_64 /dev/ttyACM2 at91sam4s4-wemo usr/historyCommand.tcl
r = subprocess.call([SAMBA_EXEC,SAMBA_RAW_DEVICE,BOARD,SAMBA_TCL])
if(r==1):
    print("all done!")
else:
    print("ERROR: bootloader failed [%d]"%r)

