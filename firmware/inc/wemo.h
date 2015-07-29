#ifndef __SERVER_H__
#define __SERVER_H__

#define PKT_SIZE 30
#define PKT_TIMESTAMP_BUF_SIZE 20
typedef struct power_pkt_struct {
  int32_t vrms[PKT_SIZE];     //RMS voltage
  int32_t irms[PKT_SIZE];     //RMS current
  int32_t watts[PKT_SIZE];    //watts
  int32_t pavg[PKT_SIZE];     //Average power (30s window)
  int32_t freq[PKT_SIZE];     //Line frequency
  int32_t pf[PKT_SIZE];       //Power factor
  int32_t kwh[PKT_SIZE];      //kWh since turn on
  char timestamp[PKT_TIMESTAMP_BUF_SIZE]; //YYYY-MM-DD HH:MM:ss
  uint8_t status;             //struct valid flag
} power_pkt;


typedef struct power_sample_struct {
  uint8_t valid;          //struct valid flag
  int32_t vrms;           //RMS voltage
  int32_t irms;           //RMS current
  int32_t watts;          //watts
  int32_t pavg;           //Average power (30s window)
  int32_t freq;           //Line frequency
  int32_t pf;             //Power factor
  int32_t kwh;            //kWh since turn on
} power_sample;

#define POWER_PKT_EMPTY      0  //all fields are empty
#define POWER_PKT_FILLING    1  //fields are being filled
#define POWER_PKT_READY      2  //all fields full, ready to send
#define POWER_PKT_TX_IN_PROG 3  //transmission in progress
#define POWER_PKT_TX_FAIL    4  //failed transmission


extern power_sample wemo_sample;


void wemo_init(void);
void wemo_read_power(void);

#endif
