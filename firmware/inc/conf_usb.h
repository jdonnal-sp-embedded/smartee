#ifndef _CONF_USB_H_
#define _CONF_USB_H_

#include "compiler.h"
#include "board.h"
#include "monitor.h"

//! Device definition (mandatory)
#define  USB_DEVICE_VENDOR_ID             USB_VID_ATMEL
#define  USB_DEVICE_PRODUCT_ID            USB_PID_ATMEL_ASF_CDC

#define  USB_DEVICE_MAJOR_VERSION         1
#define  USB_DEVICE_MINOR_VERSION         0
#define  USB_DEVICE_POWER                 100 // Consumption on Vbus line (mA)
#define  USB_DEVICE_ATTR                  \
  	(USB_CONFIG_ATTR_SELF_POWERED)
//        (USB_CONFIG_ATTR_BUS_POWERED)   
//	(USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_SELF_POWERED)
//	(USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_BUS_POWERED)

//! USB Device string definitions (Optional)
#define  USB_DEVICE_MANUFACTURE_NAME      "MIT"
#define  USB_DEVICE_PRODUCT_NAME          "WEMO Control Board"
// #define  USB_DEVICE_SERIAL_NAME           "12...EF"


/**
 * Device speeds support
 * Low speed not supported by CDC
 * @{
 */
//! To authorize the High speed
//#define  USB_DEVICE_HS_SUPPORT
//@}


/**
 * USB Device Callbacks definitions (Optional)
 * @{
 */
#define  UDC_VBUS_EVENT(b_vbus_high)
#define  UDC_SOF_EVENT()                  usb_sof_action()
#define  UDC_SUSPEND_EVENT()              usb_suspend_action()
#define  UDC_RESUME_EVENT()               usb_resume_action()
//! Mandatory when USB_DEVICE_ATTR authorizes remote wakeup feature
// #define  UDC_REMOTEWAKEUP_ENABLE()        user_callback_remotewakeup_enable()
// extern void user_callback_remotewakeup_enable(void);
// #define  UDC_REMOTEWAKEUP_DISABLE()       user_callback_remotewakeup_disable()
// extern void user_callback_remotewakeup_disable(void);

/* Configuration of CDC interface
 * @{
 */

//! Define two USB communication ports
#define  UDI_CDC_PORT_NB 1

//! Interface callback definition
#define  UDI_CDC_ENABLE_EXT(port)         usb_cdc_enable(port)
#define  UDI_CDC_DISABLE_EXT(port)        usb_cdc_disable(port)
#define  UDI_CDC_RX_NOTIFY(port)          core_read_usb(port)
#define  UDI_CDC_TX_EMPTY_NOTIFY(port)
#define  UDI_CDC_SET_CODING_EXT(port,cfg) usb_cdc_config(port,cfg)
#define  UDI_CDC_SET_DTR_EXT(port,set)    core_usb_enable(port,set)
#define  UDI_CDC_SET_RTS_EXT(port,set)

//! Define it when the transfer CDC Device to Host is a low rate (<512000 bauds)
//! to reduce CDC buffers size
//#define  UDI_CDC_LOW_RATE

//! Default configuration of communication port
#define  UDI_CDC_DEFAULT_RATE             3000000
#define  UDI_CDC_DEFAULT_STOPBITS         CDC_STOP_BITS_1
#define  UDI_CDC_DEFAULT_PARITY           CDC_PAR_NONE
#define  UDI_CDC_DEFAULT_DATABITS         8

//! The includes of classes and other headers must be done at the end of this file to avoid compile error

#include "udi_cdc_conf.h"
#include "usb.h"
#endif // _CONF_USB_H_
