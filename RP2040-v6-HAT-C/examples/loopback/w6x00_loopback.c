/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include "port_common.h"
#include "wizchip_conf.h"
#include "w6x00_spi.h"
#include "loopback.h"
#include "socket.h"
#include "stdlib.h"
#include "timer.h"

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <time.h>

#include "tcp_api.c"
// #include "tcp_server.c"
// #include "UART_Communication.c"

#include "hardware/uart.h"
#include "hardware/irq.h"

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Sockets */
#define SOCKET1_TCP_SERVER 0
#define SOCKET2_TCP_SERVER 1
#define SOCKET3_TCP_SERVER 2
#define API_SOCKET_TCP_SERVER 5

/* Ports */
#define PORT1_TCP_SERVER 5001
#define PORT2_TCP_SERVER 5002
#define PORT3_TCP_SERVER 5000
#define DEBUG_PORT_TCP_SERVER 23
#define API_PORT_TCP_SERVER 8080

#define RETRY_CNT   10000

uint8_t tcp_client_destip[] = {
    169, 254, 93, 240
};

/* Loopback */
static uint8_t g_tcp_server_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_tcp_client_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* Timers */
static uint16_t messageReceivedTimer1 = 0;
static uint16_t messageReceivedTimer2 = 0;
static uint16_t messageReceivedTimer3 = 0;

/* Clock */
static void set_clock_khz(void);
/* Timer */
static void repeating_timer_callback(void);
/* Keep alive check */
void socket_behaviour(char socket, uint16_t port, uint16_t *timer);
/* Flash memory interaction */
char readCharFromFlash(unsigned int flashAdress, char shift);

void blinkLogo(void);

char received_data2[256];  // Adjust the buffer size as needed

int main()
{
    int retval = 0;
    set_clock_khz();
    stdio_init_all();

    init_uart(UART0_ID, UART0_TX_PIN, UART0_RX_PIN);
    init_uart(UART1_ID, UART1_TX_PIN, UART1_RX_PIN); 

    //Init software uart
    gpio_init(SOFTWARE_TX_PIN);
    gpio_set_dir(SOFTWARE_TX_PIN, GPIO_OUT);

    gpio_init(SOFTWARE_RX_PIN);
    gpio_set_dir(SOFTWARE_RX_PIN, GPIO_IN);
    gpio_pull_up(SOFTWARE_RX_PIN);

    gpio_put(SOFTWARE_TX_PIN,1);

    //Standby
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);

    //Mute
    gpio_init(1);
    gpio_set_dir(1, GPIO_IN);

    //Force mute and standby
    gpio_put(0, 1);
    gpio_put(1, 1);

    //Get IP from flash
    char a = readCharFromFlash((ADDRESS + 0), 0);
    char b = readCharFromFlash((ADDRESS + 0), 8);
    char c = readCharFromFlash((ADDRESS + 0), 16);
    char d = readCharFromFlash((ADDRESS + 0), 24);
    char e = readCharFromFlash((ADDRESS + 4), 0);
    char f = readCharFromFlash((ADDRESS + 4), 8);

    //Get Baudrate from flash
    char g = readCharFromFlash(((ADDRESS + SIZE) + 0), 0);
    char h = readCharFromFlash(((ADDRESS + SIZE) + 0), 8);
    char i = readCharFromFlash(((ADDRESS + SIZE) + 0), 16);
    char j = readCharFromFlash(((ADDRESS + SIZE) + 0), 24);
    char k = readCharFromFlash(((ADDRESS + SIZE) + 4), 0);
    char l = readCharFromFlash(((ADDRESS + SIZE) + 4), 8);

    //Get DisconnectTimer from flash
    char m = readCharFromFlash(((ADDRESS + (SIZE*2)) + 0), 0);
    char n = readCharFromFlash(((ADDRESS + (SIZE*2)) + 0), 8);

    //Assign values to the correct segment of the IP
    // g_net_info.ip[0] = a;
    // g_net_info.ip[1] = b;
    // g_net_info.ip[2] = c;
    // g_net_info.ip[3] = d;

    // Convert characters from flash memory to a buatrate
    int combinedInt = 0;
    combinedInt += (g - '0') * 100000; // Assuming g represents the hundred-thousands place
    combinedInt += (h - '0') * 10000;   // Assuming h represents the ten-thousands place
    combinedInt += (i - '0') * 1000;    // Assuming i represents the thousands place
    combinedInt += (j - '0') * 100;     // Assuming j represents the hundreds place
    combinedInt += (k - '0') * 10;      // Assuming k represents the tens place
    combinedInt += (l - '0');           // Assuming l represents the units place
    BAUD_RATE_SET = combinedInt;

    // Combine timer characters into an interger from flash memory
    combinedInt = 0;
    combinedInt = (m - '0') * 10 + (n - '0');
    socketInactiveTimer = combinedInt;

    sleep_ms(1000 * 3);

    printf("==========================================================\n");
    printf("Compiled @ %s, %s\n", __DATE__, __TIME__);
    printf("==========================================================\n");

    wizchip_spi_initialize();
    wizchip_cris_initialize();
    
    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    network_initialize(g_net_info);

    printf("Start address flash memory: %08x\n", ADDRESS);
    printf("User set ip: %d.%d.%d.%d\r\n", (int)g_net_info.ip[0],(int)g_net_info.ip[1],(int)g_net_info.ip[2],(int)g_net_info.ip[3]);
    printf("User set bautrate: %c%c%c%c%c%c\r\n", g, h, i, j, k, l);
    printf("User set disconnect timer: %d\r\n", socketInactiveTimer);
    printf("User set bautrate converted: %d\r\n", BAUD_RATE_SET);

    /* Get network information */
    print_network_information(g_net_info);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ_0 = UART0_IRQ;
    int UART_IRQ_1 = UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ_0, uart_rx_interrupt);
    irq_set_enabled(UART_IRQ_0, true);

    irq_set_exclusive_handler(UART_IRQ_1, uart_rx_interrupt);
    irq_set_enabled(UART_IRQ_1, true);
    
    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART0_ID, true, false);
    uart_set_irq_enables(UART1_ID, true, false);

    init_button_settings();
    bool systemOff = true;

    /* Infinite loop */
    while (1)
    {
        // const char* data_uart0 = "Hello, UART0!\n";
        // hardware_UART_send_data(UART0_ID, data_uart0);

        // const char* data_uart1 = "Hi, UART1!\n";
        // hardware_UART_send_data(UART1_ID, data_uart1);

        //Send data for software uart
        //software_UART_send_string(SOFTWARE_TX_PIN, "0xF00xAA0x0A0x030x010x270x000x000x010x55");
        // for(int i = 0; i < 10; i++){
        //     uart_transmit_command(txbuf[i]);
        //     sleep_ms(5);
        // }
        // sleep_ms(1000);

        // software_uart_read_string(SOFTWARE_RX_PIN, received_data2, sizeof(received_data2));
        // printf("SoftwareRX: %s\n", received_data2);
        // sleep_ms(100);

        // UART_receiveData(UART0_ID, socket_to_debug);
        // sleep_ms(50); 

        // UART_receiveData(UART1_ID, socket_to_debug);
        // sleep_ms(50); 

        socket_behaviour(SOCKET1_TCP_SERVER, PORT1_TCP_SERVER, &messageReceivedTimer1);
        socket_behaviour(SOCKET2_TCP_SERVER, PORT2_TCP_SERVER, &messageReceivedTimer2);
        socket_behaviour(SOCKET3_TCP_SERVER, PORT3_TCP_SERVER, &messageReceivedTimer3);
        init_server_socket(DEBUG_SOCKET_TCP_SERVER, g_tcp_server_buf, DEBUG_PORT_TCP_SERVER);
        init_api_socket(API_SOCKET_TCP_SERVER, g_tcp_server_buf, API_PORT_TCP_SERVER);
        reset_UART_interrupt_flag();

        if(buttonState[16] == true){
            systemOff = false;
        }else{
            systemOff = true;
        }

        if(systemOff == true){
            blinkLogo();
        }
    }
}

// Read a character from the flash memory 
char readCharFromFlash(unsigned int flashAddress, char shift)
{
    char character = ((*(unsigned int *)flashAddress) >> shift) & 0xFF;
    return character;
}

/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

/* Timer */
static void repeating_timer_callback(void)
{
    //Count the time after the last message has been received
    messageReceivedTimer1++;
    messageReceivedTimer2++;
    messageReceivedTimer3++;
}

/* Initialize the socket and check for connection state */
void socket_behaviour(char socket, uint16_t port, uint16_t *timer)
{
    uint8_t *buf;
    if(init_server_socket(socket, g_tcp_server_buf, port) == 5) //Check if a client connected to the socket
    {
        if(*timer>(socketInactiveTimer*1000)) //Check if the client needs to be disconnected when the client stops sending messages
        {
            if(recv(socket, buf, 2048) == 0) //Check if a message is received
            {
                printf("\r\nDisconnected socket %d, due to inactivity\r\n", socket); //Print debug to server
                if(socket == socket_to_debug) send(DEBUG_SOCKET_TCP_SERVER, "\r\nDisconnected, due to inactivity\r\n", 33); //Print debug to debug socket
                //send(socket, "Disconnected, due to inactivity\r\n", 31); //Print debug to disconnecting socket (uncomment if needed)
                disconnect(socket);
            }
            *timer = 0; //reset timer
        }
    }else{
        *timer = 0; //reset timer
    }
}

int fadeLogo = 0;
void blinkLogo(){
    // Create a JSON object and populate it
    cJSON *root = cJSON_CreateObject();
    cJSON *buttons_array = cJSON_CreateArray();
    cJSON_AddStringToObject(root, "cmd", "fade button group");
    cJSON_AddItemToArray(buttons_array, cJSON_CreateNumber(17));
    cJSON_AddItemToObject(root, "buttons", buttons_array);
    if(fadeLogo == 0){
        cJSON_AddNumberToObject(root, "fade", 1);
        fadeLogo = 1;
    }else{
        cJSON_AddNumberToObject(root, "fade", 0);
        fadeLogo = 0;
    }

    cJSON_AddNumberToObject(root, "speed", 50);                 
    fade_button_group(root);
    cJSON_Delete(root);
}