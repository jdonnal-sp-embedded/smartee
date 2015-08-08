"""SmartEE 
    John Donnal 2015
    GPL v2 (see LICENSE)

    The Smartee object encapsulates the methods exposed by the WiFi
    interface. This is meant to be easily wrapped into your own
    interface/control scheme. See demo.py for example usage

    set_relay(ip_address,set_on)
    Control relay ON/OFF over Wifi
      params:
            ip_address: IPv4 address as a string (eg "192.168.1.4")
            set_on: boolean for relay state (true=ON, false=OFF)
      returns:
            none

    set_led(ip_address, red, green, blue, blink)
    Control RGB LED over WiFi
       params:
            ip_address: IPv4 address as a string (eg "192.168.1.4")
            red,green,blue: [0-255] for the intensity of each color
            blink: integer, 0 = no blink, otherwise delay in ms

    read_meter(ip_address)
    Read meter data over WiFi
      params:
            ip_address: IPv4 address as a string (eg "192.168.1.4")
      returns:
            meter data as dictionary if succesful or None if there
            is no data to report (new data is available every minute)

"""
import socket
import numpy as np
from calendar import timegm
import time

class Smartee:
    #structure of a data packet
    NUM_SAMPLES    = 30
    STATUS_SIZE    = 4
    TIMESTAMP_SIZE = 20
    TCP_PKT_SIZE = (7*NUM_SAMPLES*4)+STATUS_SIZE+TIMESTAMP_SIZE
    SECONDS_BTWN_SAMPLES = 2
    SECONDS_BTWN_SAMPLES = 2
    PORT_NUMBER = 1336
    
    def set_relay(self,ip_addr,value):
        #now open up a connection to the plug
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((ip_addr,Smartee.PORT_NUMBER))
        if(value=="on"):
            s.sendall('relay_on')
        elif(value=="off"):
            s.sendall('relay_off')
        s.settimeout(3.0)
        try: 
            resp = s.recv(2)
        except socket.timeout:
                print("error, no response from plug")
                return -1
        if(resp=="OK"):
            return 0
        else:
            print("bad response: %s",resp)
            return -1
        print("closing socket")
        s.close()

    def set_led(self,ip_addr,red,green,blue,blink):
        #now open up a connection to the plug
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((ip_addr,PLUG_PORT_NUMBER))
        s.sendall('set_led_%d_%d_%d_%d.'%(red,green,blue,blink))
        s.settimeout(3.0)
        try: 
            resp = s.recv(2)
        except socket.timeout:
                print("error, no response from plug")
                return -1
        if(resp=="OK"):
            return 0
        else:
            print("bad response: %s",resp)
            return -1
        print("closing socket")
        s.close()

    def read_meter(self,ip_addr,last_ts=0):
        #now open up a connection to the plug
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((ip_addr,Smartee.PLUG_PORT_NUMBER))
        except socket.error:
            print "Plug refused connection"
            return None
        s.settimeout(7.0)
        s.sendall("send_data")
        try: 
            resp = ""
            while(len(resp)<Plug.TCP_PKT_SIZE):
                resp += s.recv(Plug.TCP_PKT_SIZE)
                if(resp=="error: no data"):
                    #print("plug has no new data")
                    return None
        except socket.timeout:
            print("error, no response from plug")
            return None
        except socket.error:
            print("error, plug read error")
            return None
        return self.parse_data(resp,last_ts)

    def parse_data(self,resp,last_ts):
        #now convert the response into a data object
        data = {}
        i=0
        data["vrms"]  = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["irms"]  = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["watts"] = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["pavg"]  = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["pf"]    = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["freq"]  = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["kwh"]   = np.fromstring(resp[i:i+4*NUM_SAMPLES],dtype=np.int32)
        i+=4*NUM_SAMPLES
        data["time"] = resp[i:i+19]
        #create a UNIX timestamp from the date info (all plugs run UTC)
        try:
            utc_ts = timegm(time.strptime(data['time'],"%Y-%m-%d %H:%M:%S"))
        except ValueError:
            print "corrupt date stamp"
            return None
        #convert the data to proper units
        vrms = [float(x)/1000.0 for x in data['vrms']]
        irms = [float(x)*7.77e-6 for x in data['irms']]
        watts = [float(x)/200.0 for x in data['watts']]
        pavg = [float(x)/200.0 for x in data['pavg']]
        pf = [float(x)/1000 for x in data['pf']]
        freq = [float(x)/1000 for x in data['freq']]
        kwh = [float(x)/1000 for x in data['kwh']]
        print data['time']
        #create the numpy array to put in nilmdb
        data_size = len(vrms)
        ts_start = int(round(utc_ts*1e6))
        ts = [ts_start+Smartee.SECONDS_BTWN_SAMPLES*1e3*i for i in range(data_size)]
        db_data = np.vstack([ts,vrms,irms,watts,pavg,pf,freq,kwh]).transpose()
        #make sure this new data doesn't overlap with existing data (due to drift in the plug clock)
        overlap = 0
        while(ts[overlap]<=last_ts):
            overlap+=1
            if(overlap>=len(ts)):
                print "this data is all too old, ignoring"
                break
        db_data = db_data[overlap:]
        if(len(db_data)==0):
            return None #total overlap, no new data here :(

        return db_data
