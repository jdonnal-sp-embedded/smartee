#ifndef __WIFI_H__
#define __WIFI_H__

#define RESP_BUF_SIZE LG_BUF_SIZE
#define RESP_COMPLETE_BUF_SIZE MD_BUF_SIZE
#define WIFI_RX_BUF_SIZE XL_BUF_SIZE

//#define MAX_TRIES 3 //number of times to try joining a network

#define TX_IDLE              2
#define TX_PENDING           1
#define TX_SUCCESS           0
#define TX_ERROR            -1
#define TX_TIMEOUT          -2
#define TX_BAD_DEST_IP      -3
#define TX_ERR_MODULE_RESET -4

//WiFi receive buffer, filled by wifi uart, handled by core
char *wifi_rx_buf;
uint8_t wifi_rx_buf_full; //flag to notify main loop we have data

int wifi_init(void);
int wifi_server_start(void);
int wifi_transmit(char *url, int port, char *data);
int wifi_send_cmd(const char* cmd, const char* resp_complete, char* resp, 
		  uint32_t maxlen, int timeout);

int wifi_send_txt(int ch, const char* data);
int wifi_send_raw(int ch, const uint8_t* data, int size);

int wifi_send_ip(void); //used internally but called by monitor.c when NILM IP addr is updated

#endif
