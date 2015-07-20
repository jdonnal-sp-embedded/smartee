#!/usr/bin/python

# GPLv2 see LICENSE file

import time
import serial
import numpy as np

""""
9 Data packets
0: ???
1: ???
2: Vrms         X 1000
3: Irms         / 7.77e-6
4: Watts        X 200
5: Pavg         X 200
6: Power factor X 1000
7: Frequency    X 1000
8: kWh          X 1000
"""

WEMO="/dev/tty.usbserial-DJ0057UG"
NUM_LINES = 10

wemo  = None

def read_data(wemo):
    #align the input
    match = 0
    while(True):
        ch = wemo.read(1)
#        print("%02X"%ord(ch))
        if(ord(ch)==0xAE):
            match=1
            continue
        if(match==1):
            if(ord(ch)==0x1E):
                wemo.read(28)
                break
            else:
                match = 0
    #now we are aligned,
    #read in a chunk of data
    raw = wemo.read(30*NUM_LINES)
    data = np.fromstring(raw,'>u1')
    hdr = data[0]
    length = data[1]


    data = data.reshape(NUM_LINES,30)
    #result is an array of the register values
    result = np.zeros([NUM_LINES,9])
    curline = 0
    for line in data:
        #print the line
        #print("".join(["%02X"%x for x in line]))
        #now verify the check sum
        #checksum is the 2's complement of 
        # the sum of all bytes modulo 256
        cs =0
        for x in line[:-1]:
            cs = (cs+x)%256
        cs = (~cs)+1
        cs = cs&0xFF
        if(cs!=line[-1]):
            print "checksum error: %02X != %02X"%(cs,line[-1])
        #now extract out the 9 3-byte values of the packet
        regs = []
        vals = line[2:29]
        for i in range(9):
            v1 = vals[3*i]; v2 = vals[1+3*i]; v3 = vals[2+3*i]
            val = v3<<16 | v2<<8 | v1
            if((v3&0x80)==0x80):
                val = np.fromstring(0xFF<<24 | v3<<16 | v2<<8 | v1,'i4')
                val = val[0]
            regs.append(val)
        #just print the first reg
        result[curline,:] = regs
        curline+=1
        #print("%06X | %06X"%(regs[0],regs[1]))
    vrms = np.mean(result[:,2])*1e-3
    irms = np.mean(result[:,3])*7.77e-6
    watts = np.mean(result[:,4])*0.005
    pavg = np.mean(result[:,5])*0.005
    pf = np.mean(result[:,6])*1e-3
    freq = np.mean(result[:,7])*1e-3
    kwh = np.mean(result[:,8])*1e-3
    print "%dV %.2fI %dW %dW %.2fpf %.2fHz %.2fkWh"%(int(vrms),irms,int(watts),pavg,pf,freq,kwh)
        
#    print np.mean(result[:,0])
#    print np.mean(result[:,1])
#    print np.mean(result[:,2]) #Vrms X 1000
#    scale = 550/np.mean(result[:,3])
#    print np.mean(result[:,3])*7.77e-6
#    scale = 550/np.mean(result[:,4])
#    print 0.005*np.mean(result[:,4]) # 200 X WATTS
#    print np.mean(result[:,5])       # WATTS
#    print np.mean(result[:,6])       # Power factor X 100
#    print np.mean(result[:,7])       # Frequency
#    print np.mean(result[:,8])       # kWh

    
if __name__=="__main__":
    #open up the serial port
    wemo = serial.Serial(WEMO,9600)
    while(True):
        read_data(wemo)
