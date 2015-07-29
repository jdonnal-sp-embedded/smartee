#include <asf.h>
#include <stdio.h>
#include "string.h"
#include "wifi.h"
#include "monitor.h"
#include "conf_membag.h"
////////////////////////////////
//wifi rx and tx data structures
//
//resp_buf: uart interrupt fills this buf
//        when rx_wait==true && data_tx_status==TX_IDLE
//resp_complete_buf: short string we expect wifi module
//        to respond with before the timeout
uint8_t *resp_buf=NULL;
uint32_t resp_buf_idx = 0;
char *resp_complete_buf; //expected response of command to ESP8266

bool rx_wait = false; //we are waiting for response
bool rx_complete = false; //flag set by UART int when rx_complete_str matches

int data_tx_status = TX_IDLE;
//index into incoming data buffer (data from another server)
//   this is local to wifi module because the core only gets 
//   the buffer after the full flag is set
uint32_t wifi_rx_buf_idx = 0;

//int wifi_send_ip(void); **declared in header b/c monitor calls this when NILM IP addr changes
int wifi_send_data(int ch, const uint8_t* data, int size);

int wifi_init(void){
  uint32_t BUF_SIZE = MD_BUF_SIZE;
  uint8_t tmp;  //dummy var for flushing UART
  char *buf;
  char *tx_buf;

  //allocate static buffers if they are not NULL
  if(resp_buf==NULL){
    resp_buf=core_malloc(RESP_BUF_SIZE);
  }
  if(resp_complete_buf==NULL){
    resp_complete_buf=core_malloc(RESP_COMPLETE_BUF_SIZE);
  }
  if(wifi_rx_buf==NULL){
    wifi_rx_buf = core_malloc(WIFI_RX_BUF_SIZE);
  }
  //if we are standalone, don't do anything
  if(wemo_config.standalone){
    printf("warning: wifi_init called in standalone mode\n");
    return 0; 
  }
  //initialize the memory
  buf = core_malloc(BUF_SIZE);
  tx_buf = core_malloc(BUF_SIZE);

  //set up the UART
  static usart_serial_options_t usart_options = {
    .baudrate = WIFI_UART_BAUDRATE,
    .charlength = WIFI_UART_CHAR_LENGTH,
    .paritytype = WIFI_UART_PARITY,
    .stopbits = WIFI_UART_STOP_BITS
  };
  gpio_configure_pin(PIO_PA9_IDX, (PIO_PERIPH_A | PIO_DEFAULT));
  gpio_configure_pin(PIO_PA10_IDX, (PIO_PERIPH_A | PIO_DEFAULT));
  pmc_enable_periph_clk(ID_WIFI_UART);
  sysclk_enable_peripheral_clock(ID_WIFI_UART);
  usart_serial_init(WIFI_UART,&usart_options);
  //flush any existing data
  while(usart_serial_is_rx_ready(WIFI_UART)){
    usart_serial_getchar(WIFI_UART,&tmp);
  }
  // Trigger from timer 0
  pmc_enable_periph_clk(ID_TC0);
  tc_init(TC0, 0,                        // channel 0
	  TC_CMR_TCCLKS_TIMER_CLOCK5     // source clock (CLOCK5 = Slow Clock)
	  | TC_CMR_CPCTRG                // up mode with automatic reset on RC match
	  | TC_CMR_WAVE                  // waveform mode
	  | TC_CMR_ACPA_CLEAR            // RA compare effect: clear
	  | TC_CMR_ACPC_SET              // RC compare effect: set
	  );
  TC0->TC_CHANNEL[0].TC_RA = 0;  // doesn't matter
  TC0->TC_CHANNEL[0].TC_RC = 64000;  // sets frequency: 32kHz/32000 = 1 Hz
  NVIC_ClearPendingIRQ(TC0_IRQn);
  NVIC_SetPriority(TC0_IRQn,1);//high priority
  NVIC_EnableIRQ(TC0_IRQn);
  tc_enable_interrupt(TC0, 0, TC_IER_CPCS);
  //reset the module
  if(wifi_send_cmd("AT+RST","ready",buf,BUF_SIZE,1)==0){
    printf("Error reseting ESP8266\n");
    //free memory
    core_free(buf);
    core_free(tx_buf);
    return -1;
  }
  //set to mode STA
  if(wifi_send_cmd("AT+CWMODE=1","no change",buf,BUF_SIZE,1)==0){
    printf("Error setting ESP8266 mode\n");
    core_free(buf);
    core_free(tx_buf);
    return 0;
  }
  //try to join the specified network  
  snprintf(tx_buf,BUF_SIZE,"AT+CWJAP=\"%s\",\"%s\"",
	   wemo_config.wifi_ssid,wemo_config.wifi_pwd);
  if(wifi_send_cmd(tx_buf,"OK",buf,BUF_SIZE,10)==0){
    printf("no response to CWJAP\n");
    //free memory
    core_free(buf);
    core_free(tx_buf);
    return -1;
  }
  //check for errors
  if(strcmp(buf,"OK")!=0){
    snprintf(tx_buf,BUF_SIZE,"failed to join network [%s]: [%s]\n",wemo_config.wifi_ssid, buf);
    printf(tx_buf);
    core_log(tx_buf);
    //free memory
    core_free(buf);
    core_free(tx_buf);
    return -1;
  }
  //see if we have an IP address
  wifi_send_cmd("AT+CIFSR","OK",buf,BUF_SIZE,2);
  if(strstr(buf,"ERROR")==buf){
    printf("error getting IP address\n");
    //free the memory
    core_free(tx_buf);
    core_free(buf);
    return -1;
  }
  //try to parse the response into an IP address
  //expect 4 octets but *not* 0.0.0.0
  int a1,a2,a3,a4;
  if(!(sscanf(buf,"+CIFSR:STAIP,\"%d.%d.%d.%d\"",&a1,&a2,&a3,&a4)==4 && a1!=0)){
    printf("error, bad address: %s\n",buf);
    //free the memory
    core_free(tx_buf);
    core_free(buf);
    return -1;
  }
  //save the IP to our config
  snprintf(buf,BUF_SIZE,"%d.%d.%d.%d",a1,a2,a3,a4);
  memset(wemo_config.ip_addr,0x0,MAX_CONFIG_LEN);
  strcpy(wemo_config.ip_addr,buf);
  //set the mode to multiple connection
  wifi_send_cmd("AT+CIPMUX=1","OK",buf,BUF_SIZE,2);
  //start a server on port 1336
  wifi_send_cmd("AT+CIPSERVER=1,1336","OK",buf,BUF_SIZE,2);

  //if we know the NILM IP address, send it our IP
  if(strlen(wemo_config.nilm_ip_addr)!=0){
    if(wifi_send_ip()==TX_ERR_MODULE_RESET){
      return TX_ERR_MODULE_RESET;
    }
  } else {
    //get the NILM IP address from the manager
    //once we know the NILM address we send it ours
    core_get_nilm_ip_addr();
  }
  //log the event
  snprintf(buf,BUF_SIZE,"Joined [%s] with IP [%s]",
	   wemo_config.wifi_ssid,wemo_config.ip_addr);
  printf("\n%s\n",buf);
  core_log(buf);
  //free the memory
  core_free(tx_buf);
  core_free(buf);
  return 0;
}

int wifi_send_ip(void){
  int TX_BUF_SIZE = LG_BUF_SIZE;
  int PAYLOAD_BUF_SIZE = LG_BUF_SIZE;
  int r;
  char *tx_buf,*payload_buf;
  tx_buf = core_malloc(TX_BUF_SIZE);
  payload_buf = core_malloc(PAYLOAD_BUF_SIZE);
  snprintf(payload_buf,PAYLOAD_BUF_SIZE,
	   "{\"serial_number\":\"%s\",\"ip_addr\":\"%s\"}",
	   wemo_config.serial_number,wemo_config.ip_addr);
  snprintf(tx_buf,TX_BUF_SIZE,
	   "POST /config/plugs/update HTTP/1.1\r\n"
	   "User-Agent: WemoPlug\r\n"
	   "Host: NILM\r\nAccept:*/*\r\n"
	   "Connection: keep-alive\r\n"
	   "Content-Type: application/json\r\n"
	   "Content-Length: %d\r\n"
	   "\r\n%s",
	   strlen(payload_buf),payload_buf);
  //send the packet!
  r = wifi_transmit(wemo_config.nilm_ip_addr,80,tx_buf);
  core_free(tx_buf);
  core_free(payload_buf);
  return r;
}
int wifi_transmit(char *url, int port, char *data){
  int BUF_SIZE = MD_BUF_SIZE;
  char *cmd;
  char *buf;
  int r;
  //allocate memory
  cmd = core_malloc(BUF_SIZE);
  buf = core_malloc(BUF_SIZE);
  //sometimes the port stays open, so check for both
  //conditions
  char *success_str = "4,CONNECT";
  char *connected_str = "ALREAY CONNECT"; //sic
  //open a TCP connection on channel 4
  snprintf(cmd,BUF_SIZE,"AT+CIPSTART=4,\"TCP\",\"%s\",%d",
	  url,port);
  wifi_send_cmd(cmd,"4,CONNECT",buf,100,5);
  //check if we are able to connect to the NILM
  if(strstr(buf,"ERROR\r\nUnlink")==buf){
    printf("can't connect to NILM\n");
    core_free(cmd);
    core_free(buf);
    return TX_BAD_DEST_IP;
  }
  //if we are still connected, close and reopen socket
  if(strstr(buf,connected_str)==buf){
    wifi_send_cmd("AT+CIPCLOSE=4","Unlink",buf,100,1);
    //now try again
    wifi_send_cmd(cmd,"Linked",buf,100,2);
  }
  //check for successful link
  if(strstr(buf,success_str)!=buf){
    printf("error, setting up link\n");
    core_free(cmd);
    core_free(buf);
    return TX_ERROR;
  }
  //send the data
  if((r=wifi_send_txt(4,data))!=0){
    printf("error transmitting data: %d\n",data_tx_status);
    core_free(cmd);
    core_free(buf);
    return r;
  }
  //connection is closed *after* we receive the server's response
  //this is processed by the core and we discard the response
  //wifi_send_cmd("AT+CIPCLOSE=4","Unlink",buf,100,1);
  core_free(cmd);
  core_free(buf);
  return r; //success!
}

//**** These are the accessor functions to transmit data ***//
int wifi_send_txt(int ch, const char* data){
  return wifi_send_raw(ch,(uint8_t*)data,strlen(data));
}

int wifi_send_raw(int ch, const uint8_t* data, int size){
  int BUFFER_SIZE=1000;
  int i=0;
  int r;
  char *tx_buf;
  tx_buf = core_malloc(MD_BUF_SIZE);

  for(i=0;i<size;i+=BUFFER_SIZE){
    if(i+BUFFER_SIZE<size)
      r=wifi_send_data(ch,&data[i],BUFFER_SIZE);
    else
      r=wifi_send_data(ch,&data[i],size-i);
    if(r!=TX_SUCCESS){ //exit early
      core_free(tx_buf);
      return r; //fail!
    }
  }
  core_free(tx_buf);
  return r;
}

//*** Private method that actually sends data ***//
int wifi_send_data(int ch, const uint8_t* data, int size){
  int cmd_buf_size = MD_BUF_SIZE;
  char *cmd;
  int timeout = 7; //wait 7 seconds to transmit the data
  //allocate memory
  cmd = core_malloc(cmd_buf_size);
  snprintf(cmd,cmd_buf_size,"AT+CIPSEND=%d,%d\r\n",ch,size);
  data_tx_status=TX_PENDING;
  rx_wait=true;
  resp_buf_idx = 0;
  memset(resp_buf,0x0,RESP_BUF_SIZE);
  memset(wifi_rx_buf,0x0,WIFI_RX_BUF_SIZE); //to make debugging easier
  wifi_rx_buf_idx=0;
  usart_serial_write_packet(WIFI_UART,(uint8_t*)cmd,strlen(cmd));
  delay_ms(250); //wait for module to be ready to accept data
  usart_serial_write_packet(WIFI_UART,(uint8_t*)data,size);
  //now wait for the data to be sent
  while(timeout>0){
    //start the timer
    tc_start(TC0, 0);	  
    //when timer expires, return what we have in the buffer
    rx_wait=true; //reset the wait flag
    while(rx_wait && data_tx_status!=TX_SUCCESS);
    tc_stop(TC0,0);
    //the success flag is set *before* we receive the server's response
    //core_process_wifi_data receives the response but discards it 
    if(data_tx_status==TX_SUCCESS){
      data_tx_status=TX_IDLE;
      rx_wait = false;
      //free memory
      core_free(cmd);
      return TX_SUCCESS;
    }
    timeout--;
  }
  //check if this is a timeout error
  if(strlen((char*)resp_buf)==0){
    printf("timeout error\n");
    core_log("timeout error");
    data_tx_status = TX_TIMEOUT;
  }
  //an error occured, see if it is due to a module reset
  else if(strcmp((char*)resp_buf,"\r\nready\r\n")==0){
    //module reset itself!!! 
    printf("detected module reset\n");
    core_log("module reset");
    data_tx_status = TX_ERR_MODULE_RESET;
  } else {
    data_tx_status=TX_ERROR;
    core_log("TX error:");
    core_log((char*)resp_buf);
    data_tx_status = TX_ERROR;
  }
  //free memory
  core_free(cmd);
  return data_tx_status;
}

int wifi_send_cmd(const char* cmd, const char* resp_complete,
		  char* resp, uint32_t maxlen, int timeout){
  resp_buf_idx = 0;
  uint32_t rx_start, rx_end;
  //clear out the response buffer
  memset(resp,0x0,maxlen);
  memset(resp_buf,0x0,RESP_BUF_SIZE);
  //setup the rx_complete buffer so we know when the command is finished
  if(strlen(resp_complete)>RESP_COMPLETE_BUF_SIZE-3){
    printf("resp_complete, too long exiting\n");
    return -1;
  }
  strcpy(resp_complete_buf,resp_complete);
  strcat(resp_complete_buf,"\r\n");
  //enable RX interrupts
  usart_enable_interrupt(WIFI_UART, US_IER_RXRDY);
  NVIC_SetPriority(WIFI_UART_IRQ,2);
  NVIC_EnableIRQ(WIFI_UART_IRQ);
  //write the command
  rx_wait=true; //we want this data returned in resp_buf
  rx_complete =false; //reset the early complete flag
  usart_serial_write_packet(WIFI_UART,(uint8_t*)cmd,strlen(cmd));
  //terminate the command
  usart_serial_putchar(WIFI_UART,'\r');
  usart_serial_putchar(WIFI_UART,'\n');
  //wait for [timeout] seconds
  while(timeout>0){
    //start the timer
    tc_start(TC0, 0);	  
    //when timer expires, return what we have in the buffer
    rx_wait=true; //reset the wait flag
    while(rx_wait);
    tc_stop(TC0,0);
    if(rx_complete) //if the uart interrupt signals rx is complete
      break;
    timeout--;
  }
  //now null terminate the response
  resp_buf[resp_buf_idx]=0x0;
  //remove any ECHO
  if(strstr((char*)resp_buf,cmd)!=(char*)resp_buf){
    printf("bad echo: %s\n",resp_buf);
    return 0;
  } 
  rx_start = strlen(cmd);
  //remove leading whitespace
  while(resp_buf[rx_start]=='\r'|| resp_buf[rx_start]=='\n')
    rx_start++;
  //remove trailing whitespace
  rx_end = strlen((char*)resp_buf)-1;
  while(resp_buf[rx_end]=='\r'|| resp_buf[rx_end]=='\n')
    rx_end--;
  //make sure we have a response
  if(rx_end<=rx_start){
    printf("no response by timeout\n");
    return 0;
  }
  //copy the data to the response buffer
  if((rx_end-rx_start+1)>maxlen){
    memcpy(resp,&resp_buf[rx_start],maxlen-1);
    resp[maxlen-1]=0x0;
    printf((char*)resp_buf);
    printf("truncated output!\n");
  } else{
    memcpy(resp,&resp_buf[rx_start],rx_end-rx_start+1);
    //null terminate the response buffer
    resp[rx_end-rx_start+1]=0x0;
  }
  return rx_end-rx_start;
}

//Priority 0 (highest)
ISR(TC0_Handler)
{
  rx_wait = false;
  //clear the interrupt so we don't get stuck here
  tc_get_status(TC0,0);
}

//Priority 1 (middle)
ISR(UART0_Handler)
{
  uint8_t tmp, i;

  //state machine vars for handling RX'd data
  static int  rx_bytes_recvd = 0;
  static bool rx_in_prog = false;
  static int  rx_bytes_expected = 0;
  static int rx_chan = 0;
  char *action_buf;
  int ACTION_BUF_SIZE = MD_BUF_SIZE;

  usart_serial_getchar(WIFI_UART,&tmp);
  //if debug level is high enough, print the char
  if(wemo_config.debug_level>=DEBUG_INFO)
    core_putc(NULL,tmp);
  //check whether this is a command response or 
  //new data from the web (unsollicted response)
  if(rx_wait && data_tx_status!=TX_PENDING){
    if(resp_buf_idx>=RESP_BUF_SIZE){
      printf("error!\n");
      return; //ERROR!!!!!
    }
    resp_buf[resp_buf_idx++]=(char)tmp;
    //check for completion_str to indicate completion of command
    //this is just a speed up, we still timeout regardless
    //of whether we find this string 
    if(resp_buf_idx>4){
      if(strstr((char*)resp_buf,resp_complete_buf)==
	 (char*)&resp_buf[resp_buf_idx-strlen(resp_complete_buf)]){
	rx_complete = true; //early completion!
	rx_wait = false; 
      }
    }
  } else if(rx_wait && data_tx_status==TX_PENDING){
    //we are transmitting, no need to capture response,
    //after transmission we receive SEND OK\r\n, responses
    //are captured by wifi_rx_buf and processed by the core
    //just wait for SEND OK\r\n, use a 9 char circular buffer
    if(resp_buf_idx>8){
      for(i=0;i<8;i++)
	resp_buf[i]=resp_buf[i+1];
      resp_buf[8]=tmp;
      if(strstr((char*)resp_buf,"SEND OK\r\n")==(char*)resp_buf){
	data_tx_status=TX_SUCCESS;
      }
    } else{
      resp_buf[resp_buf_idx++]=tmp;
    }
  }else { //data_tx_status == TX_IDLE
    //this is unsollicted data, incoming from Internet,
    //process into wifi_rx_buf and pass off to the the core 
    //when the reception is complete
    if(wifi_rx_buf_full){
      printf("error, wifi_rx_buf must be processed by main loop!\n");
      return; //ERROR!!!
    }
    if(wifi_rx_buf_idx>=WIFI_RX_BUF_SIZE){
      printf("too much data, wifi_rx_buf full!\n");
      //empty the buffer
      wifi_rx_buf_idx = 0;
      return; //ERROR!!!!
    }
    //store the data in the rx buffer
    wifi_rx_buf[wifi_rx_buf_idx++]=(char)tmp;
    //if a reception is in progress...
    if(rx_in_prog){
      rx_bytes_recvd++;
      if(rx_bytes_recvd==rx_bytes_expected){
	//data is ready for processing, set flag so main loop 
	//runs core_process_wifi_data
	wifi_rx_buf_full=true;
	wifi_rx_buf_idx=0;
	rx_in_prog = false; 
	return;
      }
    }  
    //otherwise check for control sequences....
    if(wifi_rx_buf_idx>=6 && !rx_in_prog){
      //check for incoming data
      char dummy;
      if(sscanf(wifi_rx_buf,"\r\n+IPD,%d,%d:%c",&rx_chan,&rx_bytes_expected,&dummy)==3){
	rx_in_prog = true;
	rx_bytes_recvd  = 1; //reset the RX counter so we know when we have all the data
	return;
      }
      //check for link and unlink
      action_buf = core_malloc(ACTION_BUF_SIZE);
      if(sscanf(wifi_rx_buf,"%d,%s\r\n",&rx_chan,action_buf)==2){
	if(strcmp(action_buf,"CONNECT")==0){
	  core_wifi_link(rx_chan);
	  wifi_rx_buf_idx=0;
	}
	if(strcmp(action_buf,"CLOSED")==0){
	  core_wifi_unlink(rx_chan);
	  wifi_rx_buf_idx=0;
	}
      }
      //if wifi_rx_buf is longer than 20 chars we must be out of sync, look for \r\n sequence
      //and flush the buffer
      if(wifi_rx_buf_idx>20){
	if(strcmp(&wifi_rx_buf[wifi_rx_buf_idx-2],"\r\n")==0){
	  wifi_rx_buf_idx = 0; 
	  printf("flushed wifi_rx_buf, out of sync\n");
	}
      }
      core_free(action_buf);
      return;
    }
  }
}


//****NOTE: error when startup misses a character so wifi_rx_buf is out of sync**
