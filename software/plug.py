
import socket
import numpy as np
from calendar import timegm
import time
import serial

class Plug:
    #structure of packet
    NUM_SAMPLES    = 30
    STATUS_SIZE    = 4
    TIMESTAMP_SIZE = 20
    TCP_PKT_SIZE = (7*NUM_SAMPLES*4)+STATUS_SIZE+TIMESTAMP_SIZE
    SECONDS_BTWN_SAMPLES = 2
    SECONDS_BTWN_SAMPLES = 2
    PLUG_PORT_NUMBER = 1336

    def __init__(self,device,usb=False):
        #[device] should be an IPv4 address for WiFi or
        #         a device node if plug is connected by USB (eg /dev/ttyACM0)
        self.device = device
        self.usb = usb

    ######### Relay Management #############
    def set_relay(self,value):
        if(self.usb):
            self.__set_relay_usb(value)
        else:
            self.__set_relay_wifi(value)
        print "set relay [%s]"%value

    def __set_relay_usb(self,value):
        if(value!="on" and value!="off"):
            print("value must be [on|off]")
            return
        dev = serial.Serial(self.device)
        time.sleep(1.5) #wait for welcome message
        dev.write("echo off\n")
        time.sleep(0.5)
        dev.flushInput()
        time.sleep(0.5)
        dev.write("relay %s\n"%value) #LED solid green
        time.sleep(0.5)
        dev.close()
    def __set_relay_wifi(self,value):
        #now open up a connection to the plug
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.device,Plug.PLUG_PORT_NUMBER))
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

    ######### RBG LED Management #############
    def set_led(red,green,blue,blink):
        if(self.usb):
            self.__set_led_usb(red,green,blue,blink)
        else:
            self.__set_led_wifi(red,green,blue,blink)
    def __set_led_usb(self,red,green,blue,blink):
        dev = serial.Serial(self.device)
        time.sleep(1.5) #wait for welcome message
        dev.write("echo off\n")
        time.sleep(0.5)
        dev.flushInput()
        time.sleep(0.5)
        dev.write("led %d %d %d %d\n"%(red,green,blue,blink)) #LED solid green
        time.sleep(0.5)
        dev.close()
    def __set_led__wifi(self,red,green,blue,blink):
        #now open up a connection to the plug
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.device,Plug.PLUG_PORT_NUMBER))
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

    ######### Meter reading ##############
    def get_data(self,last_ts):
        if(self.usb):
            return self.__get_data_usb()
        else:
            return self.__get_data_wifi(last_ts)
    def __get_data_usb(self):
        dev = serial.Serial(self.device)
        time.sleep(1.5) #wait for welcome message
        dev.write("echo off\n")
        time.sleep(0.5)
        dev.flushInput()
        #read the plug serial number
        dev.write("config get serial_number\n")
        time.sleep(0.5)
        serial_number = dev.readline().rstrip();
        print("\tConnected to SmartEE [%s]"%serial_number)
        print("\t starting data dump")
        dev.write("led 255 255 0 500\n") #LED orange blink
        time.sleep(0.5)
        dev.write("data read\n") #start data download
        resp = dev.read(Plug.TCP_PKT_SIZE)
        if(resp=='x'*Plug.TCP_PKT_SIZE):
            time.sleep(0.5)
            dev.write("led 0 255 0 0\n") #LED solid green
            time.sleep(0.5)
            dev.close()
            return None #no data to download
        #remove overlapping data from this stream
        data = self.__parse_data(resp,0)

        last_ts = data[-1][0]
        while(resp!="x"*Plug.TCP_PKT_SIZE):
            #parse plug data into a numpy array
            res = self.__parse_data(resp,last_ts)
            if(res!=None):
                data = np.concatenate((data,res))
            resp = dev.read(Plug.TCP_PKT_SIZE)

        time.sleep(0.5)
        dev.write("led 0 255 0 0\n") #LED solid green
        time.sleep(0.5)
        dev.close()
        return data

    def __get_data_wifi(self,last_ts):
        #now open up a connection to the plug
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((self.device,Plug.PLUG_PORT_NUMBER))
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
            print("error, plug gave bad response")
            return None
        return self.__parse_data(resp,last_ts)

    def erase_data(self):
        if(not self.usb):
            print("error: cannot delete data over WiFi")
            return
        else:
            dev = serial.Serial(self.device)
            time.sleep(1.5) #wait for welcome message
            dev.write("echo off\n")
            time.sleep(0.5)
            dev.flushInput()
            time.sleep(0.5)
            dev.write("data erase\n") 
            dev.write("led 0 255 0 0\n") #LED solid green
            time.sleep(0.5)
            dev.close()
            
    def __parse_data(self,resp,last_ts):
        
        NUM_SAMPLES=Plug.NUM_SAMPLES
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
        ts_start = int(round(utc_ts*1e3))
        ts = [ts_start+Plug.SECONDS_BTWN_SAMPLES*1e3*i for i in range(data_size)]
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
