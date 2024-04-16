#include <string.h>
#include "socket.h"
#include <hardware/flash.h>
#include <hardware/sync.h>
#include "port_common.h"
#include "pico/stdlib.h"
#include <hardware/watchdog.h>
#include <ctype.h>
#include "UART_Communication.c"

#define DATA_BUF_SIZE 2048 //Max size of the data buffer
#define DEBUG_SOCKET_TCP_SERVER 3 //Socket that functions as a debug socket
#define AUDIO_SOCKET_TCP_SERVER 2
#define ADDRESS 0x1009F000  //Flash memory address
#define SIZE    4096

unsigned char buffer[SIZE];
unsigned char tempBuffer[SIZE];
unsigned int ipTemplate[] = {169, 254, 93, 240};

int socketInactiveTimer = 40; //Time that needs tot be reached in order to disconnect a client

unsigned int combineChars(char char1, char char2, char char3);
char* combineIntegersToString(int num1, int num2, int num3, int num4);
char* intToString(int num);
void mainMenu(uint8_t *buf, datasize_t len);
void networkingMenu(uint8_t *buf, datasize_t len); 
void applicationMenu(uint8_t *buf, datasize_t len);
void monitoringMenu(uint8_t *buf, datasize_t len, uint8_t owning_socket);
void printMenus(int index);
void settingsInput(void);
void print_debug_console(char message[]); 
int32_t init_server_socket(uint8_t sn, uint8_t* buf, uint16_t port);
void debug_socket_behaviour(uint8_t *buf, datasize_t len, uint8_t owning_socket);
void copyToSerial(uint8_t *buf, datasize_t len, uint8_t owning_socket);
void transmitHexCommand(uint8_t *buf);

int menuIndex = 0;
char socket_to_debug = 4; //Set tot 4 for defaulting to no socket to debug
char lastChar = 0;  

#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLCOK
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {10, 12, 99, 215},                   // IP address 169, 254, 93, 240
        .sn = {255, 255, 0, 0},                      // Subnet Mask
        .gw = {169, 254, 93, 220},                   // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .lla = {0xfe, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x02, 0x08, 0xdc, 0xff,
                0xfe, 0x57, 0x57, 0x25},             // Link Local Address
        .gua = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Global Unicast Address
        .sn6 = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // IPv6 Prefix
        .gw6 = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Gateway IPv6 Address
        .dns6 = {0x20, 0x01, 0x48, 0x60,
                0x48, 0x60, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x88, 0x88},             // DNS6 server
        .ipmode = NETINFO_STATIC_ALL
};

//Initialize server socket 
int32_t init_server_socket(uint8_t sn, uint8_t *buf, uint16_t port)
{
    int32_t ret;
    datasize_t sentsize = 0;
    int8_t status, inter;
    uint8_t tmp = 0;
    datasize_t received_size;
    uint8_t arg_tmp8;

    uint8_t dst_ip[16], ext_status;
    uint16_t dst_port;

    getsockopt(sn, SO_STATUS, &status);

    switch (status) //Check the status of the socket
    {
    case SOCK_ESTABLISHED:
        ctlsocket(sn, CS_GET_INTERRUPT, &inter);
        if (inter & Sn_IR_CON)
        {
            getsockopt(sn, SO_DESTIP, dst_ip);
            getsockopt(sn, SO_EXTSTATUS, &ext_status);

            printf("Connection on Socket: %d, Peer IP: %.3d.%.3d.%.3d.%.3d and ", sn, dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);

            getsockopt(sn, SO_DESTPORT, &dst_port);
            printf("Peer Port: %d\r\n", dst_port);

            arg_tmp8 = Sn_IR_CON;
            ctlsocket(sn, CS_CLR_INTERRUPT, &arg_tmp8);
        }
        getsockopt(sn, SO_RECVBUF, &received_size);

        if (received_size > 0)
        {
            if (received_size > DATA_BUF_SIZE)
                received_size = DATA_BUF_SIZE;
            ret = recv(sn, buf, received_size);
            
            if (ret <= 0)
                return ret; // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            received_size = (uint16_t)ret;
            sentsize = 0;

            while (received_size != sentsize)
            {
                debug_socket_behaviour(buf + sentsize, received_size - sentsize, sn); //Run debugging behaviour
                //if(sn != DEBUG_SOCKET_TCP_SERVER)ret = send(sn, buf + sentsize, received_size - sentsize); //Send the data if the socket is not the debug socket
                if (ret < 0)
                {
                    close(sn);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
        }else{
            return 5;
        }
        break;
    case SOCK_CLOSE_WAIT:
        printf("Socket: %d CloseWait\r\n", sn);
        getsockopt(sn, SO_RECVBUF, &received_size);
        if (received_size > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
        {
            if (received_size > DATA_BUF_SIZE)
                received_size = DATA_BUF_SIZE;
            ret = recv(sn, buf, received_size);

            if (ret <= 0)
                return ret; // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            received_size = (uint16_t)ret;
            sentsize = 0;

            while (received_size != sentsize)
            {
                debug_socket_behaviour(buf + sentsize, received_size - sentsize, sn); //Run debugging behaviour
                //if(sn != DEBUG_SOCKET_TCP_SERVER)ret = send(sn, buf + sentsize, received_size - sentsize); //Send the data if the socket is not the debug socket
                if (ret < 0)
                {
                    close(sn);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
        }

        if ((ret = disconnect(sn)) != SOCK_OK)
            return ret;
        printf("Socket: %d Socket Closed\r\n", sn);
        break;
    case SOCK_INIT:
        if ((ret = listen(sn)) != SOCK_OK)
            return ret;
        printf("Socket: %d Listen, TCP server loopback, port [%d] as %s\r\n", sn, port);
        break;
    case SOCK_CLOSED:
        printf("Socket: %d TCP server loopback start\r\n", sn);
        tmp = socket(sn, Sn_MR_TCP4, port, SOCK_IO_NONBLOCK);
        if (tmp != sn) /* reinitialize the socket */
        {
            printf("Socket: %d Fail to create socket.\r\n", sn);
            return SOCKERR_SOCKNUM;
        }
        printf("Socket: %d Socket opened[%d]\r\n", sn, getSn_SR(sn));
        break;
    default:
        break;
    }
    return 1;
}

//print message using the send() function to make it vissible in terminals/consoles for debugging
void print_debug_console(char message[])
{
    for (const char *ptr = message; *ptr != '\0'; ++ptr) //Check if the end of the message has been reached
    {
        uint8_t value = (uint8_t)(*ptr);
        send(3, &value, sizeof(value));
    }
}

// Function that handles the interaction for the timer menu (check input and assign the input to the timer)
void change_disconnect_timer_menu()
{
    printf("Enter time in seconds: ");
    unsigned int combinedNumber = 0;
    settingsInput();

    // Check 2 characters
    for (int i = 0; i < 2; ++i) {
        if (tempBuffer[i] >= '0' && tempBuffer[i] <= '9') combinedNumber = combinedNumber * 10 + (tempBuffer[i] - '0'); // Check for digits and combine/convert to int
        else { print_debug_console("\r\n  \x1B[31m [Non-digit character detected]\x1B[0m\r\n"); printf("\n\rNon-digit character detected \n\r"); return;}
    }

    buffer[0] = tempBuffer[0];
    buffer[1] = tempBuffer[1];

    // Print out new timer
    printf("Timer set to: %u", combinedNumber);
    print_debug_console("Timer Set to: ");
    uint8_t *new_buf = malloc(sizeof(tempBuffer) + 2);
    memcpy(new_buf, tempBuffer, sizeof(tempBuffer));
    new_buf[sizeof(tempBuffer)] = '\r';
    new_buf[sizeof(tempBuffer)+1] = '\n';
    free(new_buf);
    print_debug_console(new_buf);

    // Save the new timer value in flash memory
    print_debug_console("\r\n  \x1B[32m [Disconnect timer set, restart required]\x1B[0m\r\n");
    uint16_t ints = save_and_disable_interrupts();
    flash_range_erase(ADDRESS + (SIZE*2) - (XIP_BASE), SIZE);
    flash_range_program(ADDRESS + (SIZE*2) - (XIP_BASE), buffer, SIZE);
    restore_interrupts(ints); 
}

// Function that handles the interaction within the bautrate menu (type and save new bautrate)
void change_baudrate_menu()
{
    printf("Enter a Bautrate: ");
    unsigned int combinedNumber = 0;
    settingsInput();

    // Combine input numbers and check if the input consists of only digits
    for (int i = 0; i < 6; ++i) {
        if (tempBuffer[i] >= '0' && tempBuffer[i] <= '9') combinedNumber = combinedNumber * 10 + (tempBuffer[i] - '0'); // Combine and convert tot int
        else { print_debug_console("\r\n  \x1B[31m [Non-digit character detected]\x1B[0m\r\n"); printf("\n\rNon-digit character detected \n\r"); return;}
    }

    // List of baudrates that need to be recognised
    unsigned int baudrates[] = {300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 74880, 115200, 230400, 250000};
    for(int i = 0; i < sizeof(baudrates)/sizeof(baudrates[0]); i++) {
        if(combinedNumber == baudrates[i]) { // Check if bautrate exists in bautrates array
            for(int i = 0; i < 6; i++){
            buffer[i] = tempBuffer[i];
            }

            // Print new bautrate
            printf("Bautrate set to: %u", combinedNumber);
            print_debug_console("Bautrate Set to: ");
            uint8_t *new_buf = malloc(sizeof(tempBuffer) + 2);
            memcpy(new_buf, tempBuffer, sizeof(tempBuffer));
            new_buf[sizeof(tempBuffer)] = '\r';
            new_buf[sizeof(tempBuffer)+1] = '\n';
            free(new_buf);
            print_debug_console(new_buf);

            // Save bautrate in flash memory
            print_debug_console("\r\n  \x1B[32m [Bautrate set, restart required]\x1B[0m\r\n");
            uint16_t ints = save_and_disable_interrupts();
            flash_range_erase(ADDRESS + SIZE - (XIP_BASE), SIZE);
            flash_range_program(ADDRESS + SIZE - (XIP_BASE), buffer, SIZE);
            restore_interrupts(ints); 
            return;
        }
    }
    // Print error message
    print_debug_console("\r\n  \x1B[31m [Bautrate is not recognised]\x1B[0m\r\n"); 
    printf("\n\rBautrate is not recognised \n\r"); 
}

//function for retrieving user input and check if the enter key is pressed
void settingsInput(){
    char flag = 1;
    char indexer = 0;
    uint8_t status;
    memset(tempBuffer, 0, sizeof(tempBuffer));
    while(flag){
        ssize_t bytesRead = recv(DEBUG_SOCKET_TCP_SERVER, buffer, 20); // Retrieve input
        for (int i = 0; i < bytesRead; i++) {
            printf("%c", buffer[i]);
            if((tempBuffer[indexer] == 0))tempBuffer[indexer] = buffer[0]; indexer++; // Check if the spot is empty in the array
            if(buffer[i] == '\n')flag = 0;  // Check if the enter key is pressed and exit while loop
            if (buffer[i] == '\b'){print_debug_console("\r\n  \x1B[31m [Cancelled operation]\x1B[0m\r\n"); printf("\n\rCancelled operation \n\r"); return;} // Check backspace pressed
        }
        getsockopt(DEBUG_SOCKET_TCP_SERVER, SO_STATUS, &status); // Retrieve state of the socket
        if(status == SOCK_CLOSE_WAIT) return; // Exit loop when socket is disconnecting/closing
    }
}

//Get input from user to change networking settings
void change_ip_menu()
{
    printf("Enter an IP address: ");
    settingsInput(); //get user input

    //combine ip characters
    buffer[0] = combineChars(tempBuffer[0], tempBuffer[1], tempBuffer[2]);
    buffer[1] = combineChars(tempBuffer[4], tempBuffer[5], tempBuffer[6]);
    buffer[2] = combineChars(tempBuffer[8], tempBuffer[9], tempBuffer[10]);
    buffer[3] = combineChars(tempBuffer[12], tempBuffer[13], tempBuffer[14]);

    //check if the ip that has been put in alligns with the template
    //unsigned int tempIpTemplate[] = {169, 254, 93, 240}; //values that the user given ip needs to allign with
    unsigned int tempIpTemplate[] = {ipTemplate[0], ipTemplate[1], ipTemplate[2], ipTemplate[3]}; //values that the user given ip needs to allign with
    for(int i = 0; i < 2; i++) { // Check if first 3 numbers are equal to the template
        //Check if template is met or if the combined char is in the range
        if((buffer[i] != tempIpTemplate[i]) || (combineChars(tempBuffer[8], tempBuffer[9], tempBuffer[10]) > 255) || (combineChars(tempBuffer[12], tempBuffer[13], tempBuffer[14]) > 255)) { 
            print_debug_console("\r\n  \x1B[31m [Ip not in range]\x1B[0m\r\n"); 
            printf("\n\rIp not in range \n\r"); 
            return;
        }
    }

    printf("IP set to: %u.%u.%u.%u\n", buffer[0], buffer[1], buffer[2], buffer[3]);

    // Print new ip to telnet port
    print_debug_console("IP Set to: ");
    uint8_t *new_buf = malloc(sizeof(tempBuffer) + 2);
    memcpy(new_buf, tempBuffer, sizeof(tempBuffer));
    new_buf[sizeof(tempBuffer)] = '\r';       // Add spacing to message
    new_buf[sizeof(tempBuffer)+1] = '\n'; 
    free(new_buf);
    print_debug_console(new_buf);

    print_debug_console("\r\n  \x1B[32m [IP address set, system restart required]\x1B[0m\r\n");
    uint16_t ints = save_and_disable_interrupts();          // Disable interrupts to save to flash
    flash_range_erase(ADDRESS - XIP_BASE, SIZE);            // Erase flash 
    flash_range_program(ADDRESS - XIP_BASE, buffer, SIZE);  // Save in erased flash space
    restore_interrupts(ints);                               // Re-enable interrupts
}

//function for combining 3 chars into a integer (used for IP values)
unsigned int combineChars(char char1, char char2, char char3) {
    // Convert characters to integers
    int num1 = char1 - '0';
    int num2 = char2 - '0';
    int num3 = char3 - '0';

    // Combine into an unsigned integer
    unsigned int result = num1 * 100 + num2 * 10 + num3;

    return result;
}

//Read hex command from message and transmit using software UART
void transmitHexCommand(uint8_t *buf){
    uint8_t txbuf[27]; //27 is the max command length of the audio board protocol
    int stop_index = -1;
    // for (int i = 0; i < 27; i++) {
    //     printf("%x", buf[i]); // Print in hexadecimal format
    // }
    // // Loop trough max size of the command buffer (size indicates number of hex values)
    // for (int i = 0; i < 27; i++) {
    //     sscanf(buf + (i * 2), "%2hhx", &txbuf[i]); // Convert each pair of hexadecimal characters to uint8_t
    //     if(txbuf[i] == 85) stop_index = i; continue;// Check if 0x55 is seen (this is the end bit for the audio board's protocol)
    // }

    // //Print the converted values with the "0x" format for verification
    // for (int i = 0; i < stop_index + 1; i++) {
    //     printf("txbuf[%d] = 0x%02X\n", i, txbuf[i]);
    // }

    // Loop trough max size of the command buffer (size indicates number of hex values)
    for (int i = 0; i < 27; i++) {
        txbuf[i] = (uint8_t)buf[i]; // Assign each byte to txbuf
        if (txbuf[i] == 0x55) {
            stop_index = i;
            break; // Stop if 0x55 is found
        }
    }

    // //Print the converted values with the "0x" format for verification
    // for (int i = 0; i < stop_index + 1; i++) {
    //     printf("txbuf[%d] = 0x%02X\n", i, txbuf[i]);
    // }

    // Transmit hex command to audio board
    for(int i = 0; i < stop_index + 1; i++){
        uart_transmit_command(txbuf[i]);
        sleep_ms(10); // Add small timer to improve accuracy of the software based UART
    }
}

//function for adding a suffix to the tenlet message, to improve readablity
uint8_t* assignTelnetSuffix(uint8_t *buf, datasize_t len){
    uint8_t *new_buf = malloc(len + 2);
    memcpy(new_buf, buf, len);
    new_buf[len] = '\r';
    new_buf[len+1] = '\n';
    free(new_buf);
    return new_buf;
}

//funcion for formatting a HEX message (when using \x0f\x0A for example), so that telnet can show it correctly 
void hexToTelnet(uint8_t *buf){
    int stop_index = 0;
    // Check hex command length
    for (int i = 0; i < 27; i++) {       
        if ((uint8_t)buf[i] == 0x55) {
            stop_index = i;
            break; // Stop if 0x55 is found
        }
    }

    int len = stop_index+1; // Define the length of the buffer
    char formatted_output[len * 4]; // Assuming each byte is represented as "0xHH" and considering potential null terminators

    // Format the buffer contents into a string
    int formatted_length = 0;
    for (int i = 0; i < len; i++) {
        formatted_length += sprintf(formatted_output + formatted_length, "0x%02X", buf[i]); // formatting hex
    }

    // Allocate memory for the result as char pointer
    char *result = malloc(formatted_length + 1); // +1 for null terminator

    // Copy the formatted string into the result
    strcpy(result, formatted_output);
    //printf("Result: %s\n", result);
    free(result);
    send(3, assignTelnetSuffix(result, formatted_length), formatted_length+2);
}

//function for checking wich menu behaviour needs to be used
void debug_socket_behaviour(uint8_t *buf, datasize_t len, uint8_t owning_socket)
{
    lastChar = 0;
    //Check for hex command if the socket is assigned to the software uart (Audio Board interaction)
    if(owning_socket == AUDIO_SOCKET_TCP_SERVER){
        transmitHexCommand(buf);
    }

    //Display menu when the socket is equal to the debug socket (connection via Telnet)
    if(owning_socket == DEBUG_SOCKET_TCP_SERVER){
        lastChar = buf[len - 1];
        printMenus(menuIndex);   	
        switch(menuIndex){
        case 0:
            mainMenu(buf, len);
        break;
        case 1:
            networkingMenu(buf, len);
        break;
        case 2:
            applicationMenu(buf, len);
        break;
        case 3:
            monitoringMenu(buf, len, owning_socket);
        break;
        }
    }

    if(owning_socket == socket_to_debug) //Check if the current socket needs to copy its messages to the debug socket
    {
        // Check if the incomming message is a HEX command that needs to be formatted for telnet
        if(owning_socket != AUDIO_SOCKET_TCP_SERVER){ send(3, assignTelnetSuffix(buf, len), len+2);}
        else hexToTelnet(buf);
    }   
    copyToSerial(buf, len, owning_socket);
}

//function for sending the TCP data to the linked UART
void copyToSerial(uint8_t *buf, datasize_t len, uint8_t owning_socket){
    const char* data_uart = buf;
    if(owning_socket == 0)hardware_UART_send_data(UART0_ID, buf);
    if(owning_socket == 1)hardware_UART_send_data(UART1_ID, buf);
}

//function for checking wich menu needs to be shown on the screen (Telnet)
void printMenus(int index) {
    print_debug_console("\e[1;1H\e[2J"); // Set text color
    char* menu;
    switch (index) {
    case 0:
        menu = "\r\n=====Main Menu=====\r\n"
               "   1: Networking\r\n"
               "   2: Application\r\n"
               "   3: Monitoring\r\n"
               "   9: Save & Reboot\r\n"
               "=====================\r\n";
        break;
    case 1:
        menu = "\r\n=====Networking Menu=====\r\n"
               "   1: Set My IP Address\r\n"
               "   2: Set My Subnet Mask\r\n"
               "   3: Set My Gateway\r\n"
               "   9: <---\r\n"
               "=============================\r\n";
        break;
    case 2:
        menu = "\r\n=====Application Menu=====\r\n"
               "   1: Set Baudrate\r\n"
               "   2: Set socket disconnect timer\r\n"
               "   9: <---\r\n"
               "=============================\r\n";
        break;
    case 3:
        menu = "\r\n=====Monitoring Menu=====\r\n"
               "   1: Mirror socket 0\r\n"
               "   2: Mirror socket 1\r\n"
               "   3: Mirror socket 2\r\n"
               "   9: <---\r\n"
               "=============================\r\n";
        break;
    }
    print_debug_console(menu); //print correct menu
}

//function for main menu behaviour (open other menu according to user input)
void mainMenu(uint8_t *buf, datasize_t len){
    lastChar = buf[len - 1];
    switch(lastChar){
        case '0' + 1:
            menuIndex = 1;
            printMenus(menuIndex);
        break;
        case '0' + 2:
            menuIndex = 2;
            printMenus(menuIndex);
        break;
        case '0' + 3:
            menuIndex = 3;
            printMenus(menuIndex);
        break;
        case '0' + 9:
            //restart the system using the watchdog cooldown (triggers restart/reset)
            print_debug_console("\r\n  \x1B[32m [Rebooting system...]\x1B[0m\r\n");
            sleep_ms(2000);
            watchdog_enable(1, 1);
            while(1);
        break;
        default:
            print_debug_console("\r\n  \x1B[31m [Invalid command]\x1B[0m\r\n");
        break;
    }
}

//function that converts an integer into a string
char* intToString(int num) {
    int length = snprintf(NULL, 0, "%d", num);  // Get the length of the string
    char* str = (char*)malloc(length + 1);      // Allocate memory for the string
    snprintf(str, length + 1, "%d", num);       // Convert integer to string
    return str;
}

//function that combines 4 integers into a string
char* combineIntegersToString(int num1, int num2, int num3, int num4) {
    // Convert each integer to string
    char* str1 = intToString(num1);
    char* str2 = intToString(num2);
    char* str3 = intToString(num3);
    char* str4 = intToString(num4);

    // Calculate the total length of the combined string
    int totalLength = snprintf(NULL, 0, "%s.%s.%s.%s ", str1, str2, str3, str4);

    // Allocate memory for the combined string
    char* combinedString = (char*)malloc(totalLength + 1);

    // Combine the strings into the final string
    snprintf(combinedString, totalLength + 1, "%s.%s.%s.%s ", str1, str2, str3, str4);

    // Free the memory allocated for each individual string
    free(str1);
    free(str2);
    free(str3);
    free(str4);

    return combinedString;
}

//This function handles the networking menu behaviour (set IP)
void networkingMenu(uint8_t *buf, datasize_t len){
    lastChar = buf[len - 1];
    char *currentIpString;
    currentIpString = combineIntegersToString(g_net_info.ip[0],g_net_info.ip[1],g_net_info.ip[2],g_net_info.ip[3]);
    switch(lastChar){
        case '0' + 1:
            print_debug_console("\r\n  \x1B[32m [Type out the IP in format: 192.123.001.010]\x1B[0m\r\n");
            print_debug_console("Current IP: ");
            print_debug_console(currentIpString);
            free(currentIpString);
            print_debug_console("\r\nEnter new IP Address: ");
            change_ip_menu(); //run ip settings interaction
        break;
        case '0' + 2:
            //subnet setting
        break;
        case '0' + 3:
            //Gateway setting
        break;
        case '0' + 9:
            menuIndex = 0; //make the system use the main menu behaviour
            printMenus(menuIndex); //show main menu
        break;
        default:
            print_debug_console("\r\n  \x1B[31m [Invalid command]\x1B[0m\r\n");
        break;
    }
}

//This function handles the application menu behaviour (set timers, bautrate etc)
void applicationMenu(uint8_t *buf, datasize_t len){
    lastChar = buf[len - 1];
    switch(lastChar){
        case '0' + 1:
            print_debug_console("\r\n  \x1B[32m [Type out an existing Bautrate with leading 0's until length of 6]\x1B[0m\r\nBautrate: ");
            change_baudrate_menu(); //run bautrate settings interaction
        break;
        case '0' + 2:
            print_debug_console("\r\n  \x1B[32m [Type out socket disconnect timer with leading 0's (min 00, max 99)]\x1B[0m\r\nTime: ");
            change_disconnect_timer_menu(); //run timer settings interaction
        break;
        case '0' + 9:
            menuIndex = 0; //make the system use the main menu behaviour
            printMenus(menuIndex); //show main menu
        break;
        default:
            print_debug_console("\r\n  \x1B[31m [Invalid command]\x1B[0m\r\n");
        break;
    }
}

//This function handles the monitoring menu behaviour (Assigning correct socket to debug)
void monitoringMenu(uint8_t *buf, datasize_t len, uint8_t owning_socket){
    lastChar = buf[len - 1];
    switch(lastChar){
        case '0' + 1:
            socket_to_debug = 0; //set the correct socket to debug and receive messages from
            print_debug_console("\r\n  \x1B[32m [Debugging socket 0]\x1B[0m\r\n");
        break;
        case '0' + 2:
            socket_to_debug = 1;
            print_debug_console("\r\n  \x1B[32m [Debugging socket 1]\x1B[0m\r\n");
        break;
        case '0' + 3:
            socket_to_debug = 2;
            print_debug_console("\r\n  \x1B[32m [Debugging socket 2]\x1B[0m\r\n");
        break;
        case '0' + 9:
            socket_to_debug = 4;
            menuIndex = 0; //make the system use the main menu behaviour
            printMenus(menuIndex); //show main menu
        break;
        default:
            socket_to_debug = 4; //reset to a non excisting socket to stop debugging sockets
            print_debug_console("\r\n  \x1B[31m [Invalid command]\x1B[0m\r\n");
        break;
    }
}
#endif