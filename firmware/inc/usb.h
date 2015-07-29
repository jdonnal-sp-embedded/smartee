#ifndef __USB_H__
#define __USB_H__

//enable debugging

#define USB_DATA_PORT 0 // /dev/ttyACM0
#define USB_CTRL_PORT 1 // /dev/ttyACM1

//Call this function to set up USB
void usb_init(void);

//Callback hooks for the USB framework
void usb_suspend_action(void);
void usb_resume_action(void);
void usb_sof_action(void);

//Call back hooks for the CDC framework
bool usb_cdc_enable(uint8_t port);
bool usb_cdc_disable(uint8_t port);
void usb_cdc_set_dtr(uint8_t port, bool b_enable);
void usb_rx_notify(uint8_t port);
void usb_cdc_config(uint8_t port, usb_cdc_line_coding_t * cfg);


#endif
