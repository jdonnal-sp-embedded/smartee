#!/usr/bin/python

desc = """demo.py
    John Donnal 2015
    GPL v2 (see LICENSE)

    This demonstrates some of the capabilities of the SmartEE plug control board
    
    Usage:

    1.) Control plug relay:
        python demo.py --relay on  192.168.1.4
        python demo.py --relay off 192.168.1.4
    2.) Read meter over WiFi, appending to a data file
        python demo.py --read -f meter.dat 192.168.1.4
    3.) Download data over USB, appending to a data file
        python demo.py --read --usb -f meter.dat /dev/ttyACM0

    Data files created by this script are CSV formatted with the following columns
           ts | vrms | irms | watts | pavg | pf | freq | kwh 

    ts   | timestamp (UNIX milliseconds)
    vrms | RMS Voltage
    irms | RMS Current
    watts| Watts
    pavg | 30 second average of watts
    pf   | Power Factor
    freq | Line Frequency (Hz)
    kwh  | Energy used since plugged in (kWh)

------------------------------------------------------------------------------

"""

import socket
import os
import argparse
import csv


def set_relay(device,value,usb):
    plg = Plug.new(device,usb)
    plg.set_relay(value)

def read_meter(device,dest_file,usb):
    plg = Plug.new(device,usb)
    last_ts = 0
    #open the destination file and read the last timestamp
    with open(dest_file, newline='') as f:
        reader = csv.reader(f)
        for row in reader:
            last_ts = row[0]
    #check the plug for new data
    data = plg.get_data(last_ts)
    if(data==None):
        print("no new data available, exiting")
    else:
        #add the data to the file
        with open('some.csv', 'wa', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(data)

if __name__ == "__main__":
        parser = argparse.ArgumentParser(
            formatter_class = argparse.RawDescriptionHelpFormatter,
            description = desc)
 
        parser.add_argument("--relay", action="store", choices=["on","off"],
                           help="Set relay state")
        parser.add_argument("--read", action="store_true",
                            help="request meter data")
        parser.add_argument("--usb", action="store_true",
                            help="plug connected by USB, specify device node, \
                            *not* IPv4 address")
        parser.add_argument("--file",action="store",
                            help="destination file for meter data")
        parser.add_argument("device", action="store",
                            help="Device: either a /dev/NODE or an IPv4 address")
        
        args = parser.parse_args()
        
        #------validate the arguments-----
        #make sure user selected [relay] or [read], not both
        if(args.relay and args.read):
            print("Error, specify [read] or [relay], not both")
            exit(1)
        #make sure either [relay] or [read] has been specified
        if((args.relay==None) and (args.read==None)):
            print("Error, specify [read] or [relay]")
            exit(1)
        #if usb is not selected, make sure device is an IP address
        if(args.usb==False):
            try:
                socket.inet_aton(args.device)
            except socket.error:
                print("[%s] is not a valid IPv4 address"%args.device)
                exit(1)
        else: #make sure device is a valid descriptor
            try:
                fd = os.open(args.device,os.O_RDWR)
                if(not os.isatty(fd)):
                    print("[%s] doesn't look like a SmartEE"%args.device)
                    exit(1)
            except OSError:
                print("Cant' find [%s], file doesn't exist"%args.device)
                exit(1)

        #-----basic validation checks out, perform the requested action---
        if(args.relay):
            set_relay(args.device,args.relay)
        elif(args.read):
            read_meter(args.device,args.file,usb=args.usb)
        else:
            print("Error: no action specified (shouldn't get here...)")
            exit(1)
        
        print("All Done!")
        exit(0)

   
