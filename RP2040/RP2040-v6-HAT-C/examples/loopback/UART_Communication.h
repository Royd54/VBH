#ifndef PICO_UART_COMMUNICATION_H
#define PICO_UART_COMMUNICATION_H

#include <stdint.h>
#include "pico/stdlib.h"

//Baudrate that is used for transmitting and recieving data
#define BAUD_RATE 38400

unsigned int BAUD_RATE_SET = 38400;

//Timing calculated by 1/BAUD_RATE*1000000 for waiting the correct amount of time(in microseconds) according to the baudrate 
#define BAUD_RATE_TIMING (1e6/57600) //26 voor 38400

//uart0 ID
#define UART0_ID uart0
//uart1 ID
#define UART1_ID uart1

//GPIO TX pin for hardware based UART0
#define UART0_TX_PIN 12
//GPIO RX pin for hardware based UART0
#define UART0_RX_PIN 13

//GPIO TX pin for hardware based UART1
#define UART1_TX_PIN 8//8 
//GPIO RX pin for hardware based UART1
#define UART1_RX_PIN 9//9 

//GPIO TX pin for software based UART
#define SOFTWARE_TX_PIN 4//4
#define SOFTWARE_RX_PIN 5//5 

//Maximum data length for data that can be recieved
#define MAX_DATA_LENGTH 4096

extern void UART_receiveData(uart_inst_t *uart);
extern void init_uart(uart_inst_t *uart, int tx_pin, int rx_pin);
extern void hardware_UART_send_data(uart_inst_t *uart, const char* data);
extern void software_UART_send_bit(uint8_t gpio_pin, uint8_t bit);
extern void software_UART_send_byte(uint8_t gpio_pin, uint8_t byte);
extern void software_UART_send_string(uint8_t gpio_pin, const char *str);

#endif  // MY_UART_HEADER_H