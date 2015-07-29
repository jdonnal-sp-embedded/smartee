#include <asf.h>
#include "string.h"
#include "wemo.h"
#include "monitor.h"

/**********************************
------Theory of Operation----------
The server listens for relay commands over TCP
and pushes out updates from the power meter over TCP
-----Relay Commands-------
relay_on: turn the relay on
relay_off: turn the relay off
----Power Meter----------
Read the wemo power chip every second.  check if data in the power struct
is valid, if so send it //otherwise flush it and start listening to
the UART. The UART waits for the sync byte 0xAE then reads 29 more
bytes computes the checksum and parses the data into the power
struct if the checksum is good and everything is stored the valid
flag is set
************************************/


power_sample wemo_sample;
uint8_t wemo_buffer [30];
//take 30 byte buffer from WEMO and fill power sample struct
uint8_t process_sample(uint8_t *buffer);

void wemo_init(void){
  //allocate memory for the server buffer
  //set up the power meter UART
  static usart_serial_options_t usart_options = {
    .baudrate = WEMO_UART_BAUDRATE,
    .charlength = WEMO_UART_CHAR_LENGTH,
    .paritytype = WEMO_UART_PARITY,
    .stopbits = WEMO_UART_STOP_BITS
  };
  gpio_configure_pin(PIO_PB2_IDX, (PIO_PERIPH_A | PIO_DEFAULT));
  pmc_enable_periph_clk(ID_WEMO_UART);
  sysclk_enable_peripheral_clock(ID_WEMO_UART);
  usart_serial_init(WEMO_UART,&usart_options);
  NVIC_SetPriority(WEMO_UART_IRQ,4); //lowest priority
  NVIC_EnableIRQ(WEMO_UART_IRQ);
};


uint8_t process_sample(uint8_t *buffer){
  //process 30 byte data packet buffer
  uint8_t checksum = 0;
  uint8_t bytes[3];
  int32_t vals[9];
  int i;
  //1.) check for header and length
  if(buffer[0]!=0xAE || buffer[1]!=0x1E){
    return false;
  }
  //2.) compute checksum
  for(i=0;i<29;i++){
    checksum += buffer[i];
  }
  checksum = (~checksum)+1;
  if(checksum!=buffer[29]){
    core_log("bad checksum");
    return false;
  }

  //3.) Parse raw data into values
  //    Data is 3 byte signed LSB
  for(i=0;i<9;i++){
    bytes[0] = buffer[3*i+2];
    bytes[1] = buffer[3*i+3];
    bytes[2] = buffer[3*i+4];
    vals[i] = bytes[0] | bytes[1]<<8 | bytes[2]<<16;
    if((bytes[2]&0x80)==0x80){
      vals[i] |= 0xFF << 24; //sign extend top byte
    }
  }
  //4.) Populate the power struct
  wemo_sample.vrms = vals[2];
  wemo_sample.irms = vals[3];
  wemo_sample.watts= vals[4];
  wemo_sample.pavg = vals[5];
  wemo_sample.pf = vals[6];
  wemo_sample.freq   = vals[7];
  wemo_sample.kwh  = vals[8];
  //5.) Set the valid flag
  wemo_sample.valid = true; 
  //all done
  return true;
};

void wemo_read_power(void){
  //start listening to the WEMO
  wemo_sample.valid=false;
  usart_enable_interrupt(WEMO_UART, US_IER_RXRDY);
}

ISR(UART1_Handler)
{
  uint8_t tmp;
  static uint8_t buf[30]; //30 byte packet
  static uint8_t buf_idx=0;
  usart_serial_getchar(WEMO_UART,&tmp);
  switch(buf_idx){
  case 0: //search for sync byte
    if(tmp==0xAE){
      //found sync byte, start capturing the packet
      buf[buf_idx++]=tmp;
    }
    break;
  case 30: //sample is full, read checksum and return data
    if(process_sample(buf)){
      //success, stop listening to the UART
      usart_disable_interrupt(WEMO_UART, US_IER_RXRDY);
      //reset the index
      buf_idx=0;
    } else { //failure, look for the next packet
      buf_idx=0;
    }
    break;
  case 1: //make sure the packet length is valid
    if(tmp!=0x1E){
      buf_idx = 0;
      break;
    } //else, add to buf
  default: //reading packet
    buf[buf_idx++]=tmp;
  }
}
