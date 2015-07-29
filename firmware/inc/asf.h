#ifndef ASF_H
#define ASF_H

/*
 * This file includes all API header files for the selected drivers from ASF.
 * Note: There might be duplicate includes required by more than one driver.
 *
 */

#include <compiler.h>
#include <status_codes.h>
#include <board.h>

// Drivers
#include <sysclk.h>
#include <gpio.h>
#include <rstc.h>
#include <pmc.h>
#include <wdt.h>
#include <dacc.h>
#include <adc.h>
#include <tc.h>
#include <twi.h>
#include <delay.h>
#include <serial.h>
#include <pwm.h>
#include <gpbr.h>
#include <efc.h>


// From module: High Speed Multimedia Card Interface
#include <hsmci.h>
//From module: SD/MMC Memory Control Access - Enable
#include <sd_mmc_mem.h>
// From module: SD/MMC stack on Multimedia Card interface
#include <sd_mmc.h>
// From module: Supported ports of FatFS
#include <diskio.h>
// From module: Memory Control Access Interface
#include <ctrl_access.h>
// From module: FatFS file system
#include <diskio.h>
#include <ff.h>
#include <ffconf.h>
#include <integer.h>

// From module: USB CDC Protocol
#include <usb_protocol_cdc.h>

// From module: USB Device CDC (Single Interface Device)
#include <udi_cdc.h>

// From module: USB Device Stack Core (Common API)
#include <udc.h>
#include <udd.h>

// From module: membag
//#include <membag.h>

#endif // ASF_H
