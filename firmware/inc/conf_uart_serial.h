
#ifndef CONF_USART_SERIAL_H
#define CONF_USART_SERIAL_H

/** USART Interface */
#define WIFI_UART              ((usart_if)UART0)
#define ID_WIFI_UART           ID_UART0
#define WIFI_UART_IRQ          UART0_IRQn
/** Baudrate setting */
#define WIFI_UART_BAUDRATE     9600
/** Character length setting */
#define WIFI_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
/** Parity setting */
#define WIFI_UART_PARITY       US_MR_PAR_NO
/** Stop bits setting */
#define WIFI_UART_STOP_BITS    US_MR_NBSTOP_1_BIT

/** USART Interface */
#define WEMO_UART              ((usart_if)UART1)
#define ID_WEMO_UART           ID_UART1
#define WEMO_UART_IRQ          UART1_IRQn
/** Baudrate setting */
#define WEMO_UART_BAUDRATE     9600
/** Character length setting */
#define WEMO_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
/** Parity setting */
#define WEMO_UART_PARITY       US_MR_PAR_NO
/** Stop bits setting */
#define WEMO_UART_STOP_BITS    US_MR_NBSTOP_1_BIT

#endif/* CONF_USART_SERIAL_H_INCLUDED */
