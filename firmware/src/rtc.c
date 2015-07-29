#include <asf.h>
#include "debug.h"
#include "rtc.h" //NOTE: this is the custom RTC
#include "time.h"

//BCD helper functions
int int2bcd(int val);
int bcd2int(int val);
uint32_t date2ts(int sc, int mn, int hr, 
		 int dw, int dt, int mo, int yr);  

//initialize the i2c bus
int i2c_rtc_init(void){
  twi_options_t opt;
  uint32_t r;

  //enable peripherial mode on TWI lines
  gpio_configure_pin(PIO_PA3_IDX,(PIO_PERIPH_A | PIO_DEFAULT));
  gpio_configure_pin(PIO_PA4_IDX,(PIO_PERIPH_A | PIO_DEFAULT));

  //enable TWI clock
  pmc_enable_periph_clk(RTC_ID_TWI);
  //setup up TWI options
  opt.master_clk = sysclk_get_cpu_hz();
  opt.speed = 400000; //400kHz
  opt.smbus = 0x0;
  opt.chip = 0x0;
  if((r=twi_master_init(RTC_BASE_TWI, &opt)) != TWI_SUCCESS){
    printf("error setting up I2C for RTC: %d\n",(int)r);
    return -1;
  }
  return 0;
}
//returns unix timestamp 
void rtc_get_time(Rtc *p_rtc, uint32_t *ul_hour, uint32_t *ul_minute, uint32_t *ul_second){
  uint32_t r;
  twi_packet_t packet_rx;
  uint8_t rx_data [8];
  int sc,mn,hr;
  packet_rx.chip = RTC_ADDRESS;
  packet_rx.addr[0] = 0x0;
  packet_rx.addr_length = 1;
  packet_rx.buffer = rx_data;
  packet_rx.length = 3;
  if((r=twi_master_read(RTC_BASE_TWI, &packet_rx)) != TWI_SUCCESS){
    printf("error reading RTC: %d\n",(int)r);
    return;
  }
  //convert from BCD to decimal
  sc=bcd2int(rx_data[0]);
  mn=bcd2int(rx_data[1]);
  hr=bcd2int(rx_data[2]);
  *ul_hour = hr;
  *ul_minute = mn;
  *ul_second = sc;
  //  printf("%d:%d:%d\n",hr,mn,sc);
}

//returns timestamp as string YYYY-MM-DD HH:MM:SS
void rtc_get_time_str(char* buf, int buf_size){
  uint32_t ul_year, ul_month, ul_day, ul_week;
  uint32_t ul_hour, ul_minute, ul_second;
  rtc_get_date(NULL,&ul_year,&ul_month,&ul_day,&ul_week);
  rtc_get_time(NULL,&ul_hour,&ul_minute,&ul_second);
  snprintf(buf,buf_size,"%02lu-%02lu-%02lu %02lu:%02lu:%02lu",ul_year,ul_month,ul_day,
	  ul_hour,ul_minute,ul_second);
  return;
}


//returns the date
void rtc_get_date(Rtc *p_rtc, uint32_t *ul_year, uint32_t *ul_month, uint32_t *ul_day,
		      uint32_t *ul_week){
  uint32_t r;
  twi_packet_t packet_rx;
  uint8_t rx_data [8];
  int dw,dt,mo,yr;
  packet_rx.chip = RTC_ADDRESS;
  packet_rx.addr[0] = 0x3;
  packet_rx.addr_length = 1;
  packet_rx.buffer = rx_data;
  packet_rx.length = 4;
  if((r=twi_master_read(RTC_BASE_TWI, &packet_rx)) != TWI_SUCCESS){
    printf("error in get_date: %d\n",(int)r);
    return;
  }
  //convert from BCD to decimal
  dw=bcd2int(rx_data[0]);
  dt=bcd2int(rx_data[1]);
  mo=bcd2int(rx_data[2]);
  yr=bcd2int(rx_data[3])+2000;
  *ul_year = yr;
  *ul_month= mo;
  *ul_day = dt;
  *ul_week = dw;
  //printf("20%d/%d/%d (%d)\n",yr,mo,dt,dw);
}

//set RTC with unix timestamp
int i2c_rtc_set_time(uint8_t sc, uint8_t mn, uint8_t hr, uint8_t dw, 
		     uint8_t dt, uint8_t mo, uint8_t yr){
  //transmit the full RTC date, 7 registers
  // SC | MN | HR | DW | DT | MO | YR
  uint32_t r;
  uint8_t tx_data [8];
  twi_packet_t packet_tx;
  tx_data[0] = int2bcd(sc); //sec
  tx_data[1] = int2bcd(mn); //min
  tx_data[2] = int2bcd(hr); //hour
  tx_data[3] = int2bcd(dw); //day of week
  tx_data[4] = int2bcd(dt); //date
  tx_data[5] = int2bcd(mo); //month
  tx_data[6] = int2bcd(yr); //year
  
  packet_tx.chip = RTC_ADDRESS;
  packet_tx.addr[0] = 0x0;
  packet_tx.addr_length = 1;
  packet_tx.buffer = tx_data;
  packet_tx.length = 7;
  if((r=twi_master_write(RTC_BASE_TWI, &packet_tx)) != TWI_SUCCESS){
    printf("error setting RTC: %d\n",(int)r);
    return -1;
  }
  return 0;
}

void rtc_set_hour_mode(Rtc *p_rtc, int val){
  return; //TODO
}

////////HELP FUNCTIONS/////////////

int bcd2int(int val){
  int h,l;
  h=(val&0xF0)>>4;
  l=val&0x0F;
  return h*10+l;
}

int int2bcd(int val){
  int t,o;
  t = val/10;
  o = val%10;
  return (t<<4)|o;
}

//**from http://minirighi.sourceforge.net/html/group__RTCDriver.html **
uint32_t date2ts(int sc, int mn, int hr, 
		 int dw, int dt, int mo, int yr){
  //*******THIS DOESN'T WORK**************
  int year = yr+2000;
  uint32_t ts;
  if (0 >= (int) (mo -= 2)) {    /* 1..12 -> 11,12,1..10 */
    mo += 12;              /* Puts Feb last since it has leap day */
    year -= 1;
  }
  ts =  (((
	    (unsigned long) (year/4 - year/100 + year/400 + 367*mo/12 + dt) +
	    yr*365 - 719499
	    )*24 + hr /* now have hours */
	   )*60 + mn /* now have minutes */
	  )*60 + sc; /* finally seconds */
  return ts;
}
