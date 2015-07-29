// THIS FILE IS INCLUDED BY:
// asf/common/services/clock/sysclk.h

// XTAL frequency: 12MHz
// System clock source: PLLA
// System clock prescaler: divided by 2
// PLLA source: XTAL
// PLLA output: XTAL * 20 / 1
// System clock: 12 * 20 / 1 / 2 = 120MHz
// PLLB source: XTAL
// PLLB output: XTAL * 4 / 1
// USB Clock: PLLB = 48 MHz
#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_PLLACK
#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_2
#define CONFIG_PLL0_SOURCE          PLL_SRC_MAINCK_XTAL
#define CONFIG_PLL0_MUL             20
#define CONFIG_PLL0_DIV             1

#define CONFIG_PLL1_SOURCE          PLL_SRC_MAINCK_XTAL
#define CONFIG_PLL1_MUL             4
#define CONFIG_PLL1_DIV             1
#define CONFIG_USBCLK_SOURCE        USBCLK_SRC_PLL1
#define CONFIG_USBCLK_DIV           1
