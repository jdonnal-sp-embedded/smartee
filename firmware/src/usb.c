#include "asf.h"
#include "usb.h"
#include "conf_usb.h"
#include "debug.h"

static volatile bool b_cdc_enable = false;
#undef USB_DBG

void usb_init(void){
#ifdef USB_DBG
  print("starting usb framework\n");
#endif
  udc_start();
}

//Callback hooks for the USB framework
void usb_resume_action(void){
#ifdef USB_DBG
  print("resuming usb\n");
#endif
}
void usb_suspend_action(void){
#ifdef USB_DBG
  print("suspending usb\n");
#endif
}
void usb_sof_action(void){
  /*  if(b_cdc_enable){
    print("sof\n");
    }*/
}

//Call back hooks for the CDC framework
bool usb_cdc_enable(uint8_t port){
#ifdef USB_DBG
  print("cdc enabled\n");
#endif
  b_cdc_enable = true;
  return true;
}

bool usb_cdc_disable(uint8_t port){
#ifdef USB_DBG
  print("cdc disabled\n");
#endif
  b_cdc_enable = false;
  return true;
}

void usb_cdc_set_dtr(uint8_t port, bool b_enable){
#ifdef USB_DBG
  if(b_enable)
    print("cdc: host open\n");
  else
    print("cdc: host closed\n");
#endif
}
void usb_rx_notify(uint8_t port){
#ifdef USB_DBG
  print("cdc rx notify\n");
#endif
  //  udi_cdc_putc('A');
}
void usb_cdc_config(uint8_t port, usb_cdc_line_coding_t * cfg){
#ifdef USB_DBG
  print("cdc config\n");
#endif
}

