#ifndef __I2C_RTC_H__
#define __I2C_RTC_H__

#define RTC_ID_TWI ID_TWI0
#define RTC_BASE_TWI TWI0
#define RTC_ADDRESS 0x68

//initialize the i2c bus
int i2c_rtc_init(void);
//returns unix timestamp 

//set RTC with unix timestamp
int i2c_rtc_set_time(uint8_t sc, uint8_t mn, uint8_t hr, uint8_t dw, 
		     uint8_t dt, uint8_t mo, uint8_t yr);
void rtc_get_time_str(char* buf, int buf_size); //timestamp as string YYYY-MM-DD HH:MM:SS
/////emulate ASF RTC routines
void rtc_get_time(Rtc *p_rtc, uint32_t* ul_hour, uint32_t* ul_minute, uint32_t* ul_second);
void rtc_get_date(Rtc *p_rtc, uint32_t* ul_year, uint32_t* ul_month, uint32_t* ul_day, uint32_t* ul_week);
void rtc_set_hour_mode(Rtc *p_rtc, int val);

#endif
