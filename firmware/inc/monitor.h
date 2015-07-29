#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "wemo.h"

void monitor(void);

//Firmware version string
#define VERSION_STR "v1.1"

//general purpose backup regs and wdt
#define WDT_PERIOD       15000 //~8 secs
#define MAX_DROPPED_PACKETS 3 //~3 minutes of no contact

#define GPBR_RELAY_STATE GPBR4

//board pins
#define RELAY_PIN  PIO_PA13_IDX
#define BUTTON_PIN PIO_PB14_IDX
#define VBUS_PIN   PIO_PB13_IDX

//debug levels
#define DEBUG_ALL 5   //--not used--
#define DEBUG_INFO 4  //wifi dump
#define DEBUG_WARN 3  //warning messages about TX status
#define DEBUG_ERROR 2 //--not used--

//Wemo in-memory config datastructure
//Read from file system on boot
#define MAX_CONFIG_LEN 30
typedef struct config_struct {
  char serial_number[MAX_CONFIG_LEN]; //the plug's serial number (plugABCD)
  char wifi_ssid[MAX_CONFIG_LEN];     //the wireless network to connect to
  char wifi_pwd[MAX_CONFIG_LEN];      //the network password (may be blank)
  char str_standalone[MAX_CONFIG_LEN];//true/false: whether to connect to a network
  char mgr_url[MAX_CONFIG_LEN];       //URL of managment node (www.wattsworth.net)
  char nilm_id[MAX_CONFIG_LEN];       //ID NILM owner (nilm9F59)
  char nilm_ip_addr[MAX_CONFIG_LEN];  //cached IP address of wattsworth owner
  char ip_addr[MAX_CONFIG_LEN];       //runtime config, currently assigned IP address
  uint8_t echo;                       //runtime config, echo USB chars
  uint8_t debug_level;                //runtime config, higher level = more verbose
  bool standalone;                    //runtime boolean value of str_standalone
  bool collect_data;                  //runtime config to collect wemo data
} config;

extern config wemo_config;

// Functions implementing monitor commands
// these functions are meant to be called over
// serial, they are just wrappers for core commands
// firmware modules call the core commands directly
int mon_help(int argc, char **argv);
int mon_rtc(int argc, char **argv);
int mon_relay(int argc, char **argv);
int mon_echo(int argc, char **argv);
int mon_config(int argc, char **argv);
int mon_meter(int argc, char **argv);
int mon_log(int argc, char **argv);
int mon_data(int argc, char **argv);
int mon_restart(int argc, char **argv);
int mon_memory(int argc, char **argv);
int mon_wifi(int argc, char **argv);
int mon_debug(int argc, char **argv);
int mon_version(int argc, char **argv);
int mon_led(int argc, char **argv);
int mon_ls(int argc, char **argv);
int mon_collect_data(int argc, char **argv);

//putc for stdout
void core_putc(void* stream, char c);


//Functions implementing core commands

//   Incoming data from WiFi
void core_process_wifi_data(void); // main loop
void core_wifi_link(int ch);         // interrupt ctx
void core_wifi_unlink(int ch);       // interrupt ctx
//   Outgoing data to WiFi
void core_get_nilm_ip_addr(void);
void core_get_nilm_ip_addr_cb(char* data); //callback
void core_log_power_data(power_sample *data);
void core_transmit_power_packets(void);
//   System logging
void core_log(const char* content);
//   USB terminal functions
void core_read_usb(uint8_t port);
void core_usb_enable(uint8_t port, bool b_enable);
//   Panic loop
void core_panic(void);
//   Allocate, clear, and panic on failure
void* core_malloc(int size);
//   Free (wrapper around membag_free)
void core_free(void* ptr);
#endif
