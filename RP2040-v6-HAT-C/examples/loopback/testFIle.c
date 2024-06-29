#include <stdio.h>
#include "UART_Communication.c"

char software_uart_read(uint8_t gpio_pin);
void software_uart_read_string(uint8_t gpio_pin, char *buffer, size_t buffer_size);
char received_data2[256];  // Adjust the buffer size as needed


// int main() {
//     //Init uarts
//     stdio_init_all();
//     init_uart(UART0_ID, UART0_TX_PIN, UART0_RX_PIN);
//     init_uart(UART1_ID, UART1_TX_PIN, UART1_RX_PIN); 

//     //Init software uart
//     gpio_init(SOFTWARE_TX_PIN);
//     gpio_set_dir(SOFTWARE_TX_PIN, GPIO_OUT);

//     gpio_init(SOFTWARE_RX_PIN);
//     gpio_set_dir(SOFTWARE_RX_PIN, GPIO_IN);
//     //gpio_pull_down(SOFTWARE_RX_PIN);

//     while (1) {
//         //Send data with correct function
//         const char* data_uart0 = "Hello, UART0!\n";
//         hardware_UART_send_data(UART0_ID, data_uart0);

//         const char* data_uart1 = "Hi, UART1!\n";
//         hardware_UART_send_data(UART1_ID, data_uart1);

//         //Send data for software uart
//         //software_UART_send_string(SOFTWARE_TX_PIN, "Hello, Software UART!\n");
//         software_UART_send_byte(SOFTWARE_TX_PIN, 'A');
//         sleep_ms(100);

//         // char received_data = software_uart_read(SOFTWARE_RX_PIN);
//         // printf("Received: %c\n", received_data);
//         // sleep_ms(100);

//         software_uart_read_string(SOFTWARE_RX_PIN, received_data2, sizeof(received_data2));
//         printf("SoftwareRX: %s\n", received_data2);
//         sleep_ms(100);

//         UART_receiveData(UART0_ID); //Recieve data on given uart
//         sleep_ms(100);

//         UART_receiveData(UART1_ID);
//         sleep_ms(100); 
//     }

//     return 0;
// }

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