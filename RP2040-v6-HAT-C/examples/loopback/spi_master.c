#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "tcp_server.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

// MCP2515 Commands
#define RESET_CMD       0xC0
#define READ_CMD        0x03
#define WRITE_CMD       0x02
#define BIT_MODIFY_CMD  0x05
#define RTS_TX0         0x81

// MCP2515 Registers
#define CANCTRL         0x0F
#define CANSTAT         0x0E
#define CNF1            0x2A
#define CNF2            0x29
#define CNF3            0x28
#define TXB0CTRL        0x30
#define TXB0SIDH        0x31
#define TXB0SIDL        0x32
#define TXB0DLC         0x35
#define TXB0D0          0x36
#define RXB0CTRL        0x60
#define RXB0SIDH        0x61
#define RXB0SIDL        0x62
#define RXB0DLC         0x65
#define RXB0D0          0x66
#define CANINTF         0x2C
#define RX0IF           0x01

// GPIO Pins
#define CS_PIN   13
#define INT_PIN  14

void cs_select() {
    gpio_put(CS_PIN, 0);
}

void cs_deselect() {
    gpio_put(CS_PIN, 1);
}

void mcp2515_reset() {
    cs_select();
    spi_write_blocking(spi1, (const uint8_t[]){RESET_CMD}, 1);
    cs_deselect();
    sleep_ms(10); // Give MCP2515 time to reset
}

void mcp2515_write(uint8_t address, uint8_t data) {
    cs_select();
    uint8_t buf[] = {WRITE_CMD, address, data};
    spi_write_blocking(spi1, buf, 3);
    cs_deselect();
}

void mcp2515_write_array(uint8_t address, const uint8_t *data, uint8_t length) {
    cs_select();
    uint8_t buf[length + 2];
    buf[0] = WRITE_CMD;
    buf[1] = address;
    memcpy(&buf[2], data, length);
    spi_write_blocking(spi1, buf, length + 2);
    cs_deselect();
}

uint8_t mcp2515_read(uint8_t address) {
    cs_select();
    uint8_t buf[] = {READ_CMD, address, 0x00};
    spi_write_read_blocking(spi1, buf, buf, 3);
    cs_deselect();
    return buf[2];
}

void mcp2515_read_array(uint8_t address, uint8_t *data, uint8_t length) {
    cs_select();
    uint8_t buf[length + 2];
    buf[0] = READ_CMD;
    buf[1] = address;
    spi_write_read_blocking(spi1, buf, buf, length + 2);
    memcpy(data, &buf[2], length);
    cs_deselect();
}

void mcp2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data) {
    cs_select();
    uint8_t buf[] = {BIT_MODIFY_CMD, address, mask, data};
    spi_write_blocking(spi1, buf, 4);
    cs_deselect();
}

void mcp2515_set_normal_mode() {
    mcp2515_write(CANCTRL, 0x00); // Set normal mode
}

void mcp2515_setup() {
    printf("Resetting MCP2515...\n");
    mcp2515_reset();

    printf("Writing configuration...\n");
    // Configuration settings (500 kbps with 8 MHz oscillator)
    mcp2515_write(CNF1, 0x00); // SJW = 1, BRP = 1
    mcp2515_write(CNF2, 0x90); // BTLMODE = 1, PHSEG1 = 3, PRSEG = 1, SAM = 1
    mcp2515_write(CNF3, 0x02); // PHSEG2 = 2

    // Enable interrupts on RX0
    mcp2515_write(0x2B, 0x01); // CANINTE register, enable RX0 interrupt

    // Set normal mode
    mcp2515_set_normal_mode();
}

void can_send_message() {
    printf("Sending CAN message...\n");
    // Set ID
    mcp2515_write(TXB0SIDH, 0x10);
    mcp2515_write(TXB0SIDL, 0x00);

    // Set Data Length Code (DLC)
    mcp2515_write(TXB0DLC, 0x08); // Eight bytes of data

    // Set Data with random values
    uint8_t data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = rand() % 256;
    }
    mcp2515_write_array(TXB0D0, data, 8);

    // Request to send
    cs_select();
    spi_write_blocking(spi1, (const uint8_t[]){RTS_TX0}, 1);
    cs_deselect();
}

void can_send_message_buffer(uint8_t *buf, uint8_t receiverID, uint8_t len) {
    if (len > 8) {
        printf("Error: Data length cannot be greater than 8 bytes.\n");
        return;
    }

    printf("Sending CAN message to receiver ID 0x%02X...\n", receiverID);

    // Set ID
    mcp2515_write(TXB0SIDH, (receiverID >> 3));  // Higher bits of ID
    mcp2515_write(TXB0SIDL, (receiverID << 5));  // Lower bits of ID

    // Set Data Length Code (DLC)
    mcp2515_write(TXB0DLC, len); // Data length

    // Set Data with provided buffer
    mcp2515_write_array(TXB0D0, buf, len);

    // Request to send
    cs_select();
    spi_write_blocking(spi1, (const uint8_t[]){RTS_TX0}, 1);
    cs_deselect();
}

void can_receive_message() {
    printf("Checking for CAN message...\n");
    // Check if message received
    uint8_t canintf = mcp2515_read(CANINTF);
    if (canintf & RX0IF) { // Check for message in RXB0
        printf("Message Received!\n");

        uint8_t id_high = mcp2515_read(RXB0SIDH);
        uint8_t id_low = mcp2515_read(RXB0SIDL);
        uint16_t id = ((id_high << 3) | (id_low >> 5)) & 0x7FF; // Combine ID parts correctly and mask to 11 bits
        printf("ID: 0x%X\n", id);

        uint8_t dlc = mcp2515_read(RXB0DLC) & 0x0F;
        printf("DLC: %d\n", dlc);

        uint8_t data[8];
        mcp2515_read_array(RXB0D0, data, dlc);

        printf("Data: ");
        for (int i = 0; i < dlc; i++) {
            printf("0x%02X ", data[i]);
        }
        printf("\n");
        
        // Create a character array to hold the formatted data
        char formattedData[dlc * 3 + 1]; // Each byte will be represented by up to 3 characters plus a null terminator
        char* ptr = formattedData;
        
        for (int i = 0; i < dlc; i++) {
            ptr += sprintf(ptr, "%d", data[i]); // Convert each byte to its numeric string representation
        }
        formattedData[dlc * 3] = '\0'; // Null-terminate the string

        if (menuIndex == 3 && socket_to_debug == 4) {
            send(3, formattedData, strlen(formattedData));
            send(3, "\r\n", 4);
        } else {
            send(0, formattedData, strlen(formattedData));
        }
        // Clear the received message from the buffer
        mcp2515_bit_modify(CANINTF, RX0IF, 0); // Clear RX0IF bit
    } else {
        printf("No message received\n");
    }
}

void can_interrupt_handler(uint gpio, uint32_t events) {
    if (gpio == INT_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        printf("Interrupt triggered on GPIO %d with event %d\n", gpio, events);
        can_receive_message();
    }
}

int can_setup() {
    stdio_init_all(); // Initialize stdio for printing

    // Initialize SPI
    spi_init(spi1, 1000 * 1000);
    gpio_set_function(10, GPIO_FUNC_SPI); // SCK for SPI1
    gpio_set_function(11, GPIO_FUNC_SPI); // MOSI for SPI1
    gpio_set_function(12, GPIO_FUNC_SPI); // MISO for SPI1

    // Initialize CS and INT pins
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);

    gpio_init(INT_PIN);
    gpio_set_dir(INT_PIN, GPIO_IN);
    gpio_pull_up(INT_PIN);

    // Setup interrupt
    gpio_set_irq_enabled_with_callback(INT_PIN, GPIO_IRQ_EDGE_FALL, true, &can_interrupt_handler);

    printf("Setting up MCP2515...\n");
    mcp2515_setup();

    // Initialize random seed
    srand(time(NULL));

    return 0;
}