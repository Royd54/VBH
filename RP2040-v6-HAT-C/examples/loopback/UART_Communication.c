#include <stdio.h>
#include "hardware/uart.h"
#include "UART_Communication.h"
#include "socket.h"
#include "stdint-gcc.h"
#include "tcp_api.h"

int received_data_index = 0;        // Index to keep track of the current position in the buffer
char received_data_interrupt[256];  // Define a buffer to store received data
int UART_INTERRUPTED = 0;           // Global var to check if the UART got interrupted

unsigned int BAUD_RATE_SET = 38400;
unsigned int UART_TO_DEBUG;

void uart_rx_interrupt() {
    uint8_t data;
    uart_inst_t *triggeredBy = NULL;
    int uart_index = 0;

    // check which uart is receiving a message
    if (uart_is_readable(UART0_ID)) { 
        triggeredBy = UART0_ID;
        uart_index = 0;
    }
    if (uart_is_readable(UART1_ID)) {
        triggeredBy = UART1_ID;
        uart_index = 1;
    }

    while (uart_is_readable(triggeredBy)) {
        char receivedChar = uart_getc(triggeredBy);
        if ((receivedChar == '\n' || receivedChar == '\r')) {
            if(received_data_index > 0){
            UART_INTERRUPTED = 1;
            received_data_interrupt[received_data_index++] = receivedChar;
            received_data_interrupt[received_data_index++] = '\n';
            received_data_interrupt[received_data_index++] = '\0';
            getButtonState(received_data_interrupt, uart_index);
            //send(uart_index, received_data_interrupt, received_data_index);
            //if(UART_TO_DEBUG == uart_index)send(3, received_data_interrupt, received_data_index);
            }
            received_data_index = 0;  //Reset
        } else {
            received_data_interrupt[received_data_index++] = receivedChar;
            if (received_data_index >= MAX_DATA_LENGTH - 1) {//was -1
                received_data_index = 0;  //Reset 
            }
        }
    }
}

// reset the UART_INTERRUPTED var
void reset_UART_interrupt_flag(){
    if(UART_INTERRUPTED == 1){
        if (!uart_is_readable(UART0_ID)) {
            UART_INTERRUPTED = 0;
        }
        if (!uart_is_readable(UART1_ID)) {
            UART_INTERRUPTED = 0;
        }
    }
}

//Funcion for creating uarts and settings
void init_uart(uart_inst_t *uart, int tx_pin, int rx_pin) {
    uart_init(uart, BAUD_RATE_SET);
    //uart_init(uart, BAUD_RATE);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart, 0, 0);
    uart_set_format(uart, 8, 1, UART_PARITY_NONE); //8 data bits and 1 stop bit
}

//Function for sending data trough hardware based uart
void hardware_UART_send_data(uart_inst_t *uart, const char* data) {
    // printf("Sending command: %s\n", data);
    uart_puts(uart, data);
    // printf("Command sent.\n");
}

//Function for creating a us delay using busy_wait_us()
void delay_us(uint32_t us) {
    busy_wait_us(us);
}

//Function for creating a us delay using a busy wait loop. This is more accurate then the delay_us function's method
void delay_us_busy_waiting(uint32_t us) {
    uint32_t start_time = time_us_32();
    while (time_us_32() - start_time < us);
}

//Function for transmitting HEX commands trough the GPIO pins of the Software UART
void uart_transmit_command(uint8_t data) {
    // Start bit
    gpio_put(SOFTWARE_TX_PIN, 0);
    delay_us_busy_waiting(BAUD_RATE_TIMING_PRECISE);

    // 8 data bits, LSB first
    for (int i = 0; i < 8; i++) {
        gpio_put(SOFTWARE_TX_PIN, (data >> i) & 1);
        delay_us_busy_waiting(BAUD_RATE_TIMING_PRECISE);
    }

    // Stop bit
    gpio_put(SOFTWARE_TX_PIN, 1);
    delay_us_busy_waiting(BAUD_RATE_TIMING_PRECISE);
}

//Function for receiving HEX commands using the GPIO pins of the software UART
uint8_t uart_receive_command() {
    // Wait for the start bit
    while (gpio_get(SOFTWARE_RX_PIN) == 1);

    // Wait for half a bit to sample in the middle
    delay_us(BAUD_RATE_TIMING / 2);

    uint8_t data = 0;

    // 8 data bits, LSB first
    for (int i = 0; i < 8; i++) {
        data |= (gpio_get(SOFTWARE_RX_PIN) << i);
        delay_us(BAUD_RATE_TIMING);
    }

    // Wait for stop bit
    while (gpio_get(SOFTWARE_RX_PIN) == 1);

    return data;
}

//Function for sending data trough software based uart (pin binary) trough software uart
void software_UART_send_bit(uint8_t gpio_pin, uint8_t bit) {
    gpio_put(gpio_pin, bit);
    //printf("%d",bit);
    sleep_us(BAUD_RATE_TIMING);  //Timing with bautrate 1/bautrate * 1000000
}

//Function for sending bytes (start, data, stop bits) trough software uart
void software_UART_send_byte(uint8_t gpio_pin, uint8_t byte) {
    //Start bit
    software_UART_send_bit(gpio_pin, 0);

    //Data bits
    for (int i = 0; i < 8; i++) {
        software_UART_send_bit(gpio_pin, (byte >> i) & 1);
    }

    //Stop bit
    software_UART_send_bit(gpio_pin, 1);
    sleep_us(BAUD_RATE_TIMING);  //Timing with bautrate 1/bautrate * 1000000
}

//Function for sending strings trough software uart
void software_UART_send_string(uint8_t gpio_pin, const char *str) {
    //Loop trough chars and check for end of line
    for (const char *c = str; *c != '\0'; ++c) {
        //Send string per char
        software_UART_send_byte(gpio_pin, *c);
        //printf("%c", *c);
    }
}

//Function for handeling data
void UART_receiveData(uart_inst_t *uart, char socket_to_debug) {
    char received_data[MAX_DATA_LENGTH];
    int data_index = 0; //Char index from data
    int uart_index = 0; //Int for making the used uart visible

    if(uart == uart0) uart_index = 0;
    if(uart == uart1) uart_index = 1;

    while (uart_is_readable(uart)) {
        char receivedChar = uart_getc(uart);
        if ((receivedChar == '\n' || receivedChar == '\r')) {
            if(data_index > 0){
            received_data[data_index++] = receivedChar;
            received_data[data_index++] = '\n';
            send(uart_index, received_data, data_index);                        // send uart data to the connected socket
            if(socket_to_debug == uart_index)send(3, received_data, data_index);// send uart data to the debug socket if needed
            }
            data_index = 0;  //Reset
        } else {
            received_data[data_index++] = receivedChar;
            if (data_index >= MAX_DATA_LENGTH - 1) {//was -1
                data_index = 0;  //Reset 
            }
        }
    }
}

// Function for reading a chararaacter using software UART
char software_uart_read(uint8_t gpio_pin) {
    char data = 0;
    uint32_t timeout_ms = 1000;  // Set timeout value in milliseconds
    uint32_t start_time = time_us_32();

    // Wait for the start bit
    while (gpio_get(gpio_pin)){
        if ((time_us_32() - start_time) > (timeout_ms * 1000)) {
            // Timeout reached, exit the loop
            break;
        }
    } 

    // Wait for the middle of the start bit
    sleep_us(13);

    // Read each bit, LSB first
    for (int i = 0; i < 8; i++) {
        // Wait for the next bit
        sleep_us(26);

        // Shift the current data byte to the left
        data |= gpio_get(gpio_pin) << i;
    }

    // Wait for the stop bit
    sleep_us(26);
    return data;
}

//Function for reading a full string using software UART
void software_uart_read_string(uint8_t gpio_pin, char *buffer, size_t buffer_size) {
    size_t index = 0;
    char received_char;
    uint32_t timeout_ms = 1000;  // Set timeout value in milliseconds
    uint32_t start_time = time_us_32();

    while (1) {
        received_char = software_uart_read(gpio_pin);

        if ((time_us_32() - start_time) > (timeout_ms * 1000)) {
            buffer = 0;
            break;
        }

        // Check for newline character as the end of the string
        if (received_char == '\n' || received_char == '\r') {
            buffer[index] = '\0'; // Null-terminate the string
            break;
        }

        // Store the character in the buffer
        if (index < buffer_size - 1) {
            buffer[index] = received_char;
            index++;
        }
    }
        // for (size_t i = 0; received_data2[i] != '\0'; i++) {
        //     printf("%c ", received_data2[i]);
        // }
        //  printf("\n");
}