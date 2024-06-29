#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "socket.h"

#define DATA_BUF_SIZE 2048 // Max size of the data buffer
#define DEBUG_SOCKET_TCP_SERVER 3 // Socket that functions as a debug socket
#define AUDIO_SOCKET_TCP_SERVER 2
#define ADDRESS 0x1009F000  // Flash memory address
#define SIZE    4096

extern unsigned char buffer[SIZE];
extern unsigned char tempBuffer[SIZE];
extern unsigned int ipTemplate[];

extern int socketInactiveTimer;
extern int menuIndex;
extern char socket_to_debug;
extern char lastChar;

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
extern int32_t init_server_socket(uint8_t sn, uint8_t* buf, uint16_t port);
void debug_socket_behaviour(uint8_t *buf, datasize_t len, uint8_t owning_socket);
void copyToSerial(uint8_t *buf, datasize_t len, uint8_t owning_socket);
void transmitHexCommand(uint8_t *buf);

/* Network */
static wiz_NetInfo g_net_info;
#endif
