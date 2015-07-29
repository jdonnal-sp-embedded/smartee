#ifndef __WEMO_FS__
#define __WEMO_FS__


#define CONFIG_FILE "config.txt"
#define LOG_FILE    "log.txt"
#define DATA_FILE "power.dat"

uint8_t fs_init(void);
//char* wemo_fs_get_config(const char* key);
//void wemo_fs_set_config(const char* key, const char* value);
void fs_write_config(void);
void fs_read_config(void);
void fs_write_power_pkt(const power_pkt* pkt);
void fs_erase_data(void);
void fs_log(const char* content);
void fs_info(void);
#endif
