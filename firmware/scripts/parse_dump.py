#!/usr/bin/python

#seems like chip is 78m6610+PSU (maxim)

with open('full_dump_vals.csv') as f:
    while(True):
        row = []
        for i in range(30):
            val = f.readline()
            x = int(val,16)
            row.append(x)
        #calculate the checksum
        cs = 0
        for i in range(30):
            cs += row[i]
#        cs = cs%0xFF
#        cs = 0xFF & cs
        cs = (~(cs&0xFF)+1)
        cs = 0xFF & cs

        vrms_ar = row[2:5]
        print "".join(["%02X"%x for x in row])
        print "\t vrms: %s"%"".join(["%02X"%x for x in vrms_ar])
        print "\t cs: %x"%cs

