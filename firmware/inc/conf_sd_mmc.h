/**
 * \file
 *
 * \brief SD/MMC stack configuration file.
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_SD_MMC_H_INCLUDED
#define CONF_SD_MMC_H_INCLUDED

#define SD_MMC_HSMCI_MEM_CNT          1
// Card Detect
#    define SD_MMC_0_CD_GPIO              (PIO_PA19_IDX)
#    define SD_MMC_0_CD_PIO_ID            ID_PIOA
#    define SD_MMC_0_CD_FLAGS             (PIO_INPUT | PIO_PULLUP)
#    define SD_MMC_0_CD_DETECT_VALUE      0

#define SD_MMC_HSMCI_SLOT_0_SIZE      4 // 4-bits connector pin
#define PIN_HSMCI_MCCDA_GPIO          (PIO_PA28_IDX)
#define PIN_HSMCI_MCCDA_FLAGS         (PIO_PERIPH_C | PIO_DEFAULT)
#define PIN_HSMCI_MCCK_GPIO           (PIO_PA29_IDX)
#define PIN_HSMCI_MCCK_FLAGS          (PIO_PERIPH_C | PIO_DEFAULT)
#define PIN_HSMCI_MCDA0_GPIO          (PIO_PA30_IDX)
#define PIN_HSMCI_MCDA0_FLAGS         (PIO_PERIPH_C | PIO_DEFAULT)
#define PIN_HSMCI_MCDA1_GPIO          (PIO_PA31_IDX)
#define PIN_HSMCI_MCDA1_FLAGS         (PIO_PERIPH_C | PIO_DEFAULT)
#define PIN_HSMCI_MCDA2_GPIO          (PIO_PA26_IDX)
#define PIN_HSMCI_MCDA2_FLAGS         (PIO_PERIPH_C | PIO_DEFAULT)
#define PIN_HSMCI_MCDA3_GPIO          (PIO_PA27_IDX)
#define PIN_HSMCI_MCDA3_FLAGS         (PIO_PERIPH_C | PIO_DEFAULT)

/* Define it to enable the SPI mode instead of Multimedia Card interface mode */
//#define SD_MMC_SPI_MODE

/* Define it to enable the SDIO support */
//#define SDIO_SUPPORT_ENABLE

/* Define it to enable the debug trace to the current standard output (stdio) */
//#define SD_MMC_DEBUG

#endif /* CONF_SD_MMC_H_INCLUDED */

