#include "socket.h"
#include "../libraries/cJSON/cJSON.h"
#include "tcp_server.c"
#include "tcp_api.h"

#define DATA_BUF_SIZE 2048 // Max size of the data buffer

void api_socket_behaviour(uint8_t *buf, datasize_t len, char *item);
void api_command(const char *command, cJSON *object);
void handle_unknown_command();
void button_fade_command(cJSON *object);
void handle_command2(cJSON *object);
void handle_command3(cJSON *object);
void fade_gui(cJSON *object);
void press_button(cJSON *object);
void fade_button_group(cJSON *object);
void set_button_brightness(cJSON *object);
void set_button_brightness_group(cJSON *object);
void system_restart(cJSON *object);
void disconnect_socket(cJSON *object);
void calibrate_buttons(cJSON *object);
void set_button_color(cJSON *object);
void set_button_color_group(cJSON *object);
void play_sound(cJSON *object);
void set_haptic_intensity(cJSON *object);
void update_system_ip(cJSON *object);
void update_system_baudrate(cJSON *object);
void update_socket_inactive_timer(cJSON *object);

void checkDependancy(char buttonIndex, bool state);
// void getButtonState(char command[]);
void setButtonActivity();
void playSoundEffect(int sound, int volume);

// Structure to represent a command and its handler function
typedef struct {
    const char *command_name;
    void (*handler)();
} CommandHandler;

// Array of command handlers
CommandHandler command_handlers[] = {
    {"button fade", button_fade_command},                           // {"cmd":"button fade", "fade":9, "speed":50, "button":1}
    {"cmd2", handle_command2},                                      // {"cmd":"cmd2", "data":{"id":10, "name":"test"}}
    {"cmd3", handle_command3},                                      // {"cmd":"cmd3", "buttons":[1,2,3,100,200]}
    {"fade gui", fade_gui},                                         // {"cmd":"fade gui", "fade":9, "speed":50}
    {"press button", press_button},                                 // {"cmd":"press button", "button": 10}
    {"fade button group", fade_button_group},                       // {"cmd":"fade button group", "buttons": [1,2,10,17], "fade": 1, "speed": 50}
    {"set button brightness", set_button_brightness},               // {"cmd":"set button brightness", "button": 5, "brightness": 20}
    {"set button brightnesses", set_button_brightness_group},       // {"cmd":"set button brightnesses", "buttons": [1,17,3,8], "brightness": 20}
    {"restart system", system_restart},                             // {"cmd":"restart system"}
    {"disconnect socket", disconnect_socket},                       // {"cmd":"disconnect socket", "socket": 5}
    {"calibrate buttons", calibrate_buttons},                       // {"cmd":"calibrate buttons"}
    {"set button color", set_button_color},                         // {"cmd":"set button color", "button": 5, "color": "green"}
    {"set button colors", set_button_color_group},                  // {"cmd":"set button colors", "buttons": [1,10,7,13], "color": "green"}
    {"play sound", play_sound},                                     // {"cmd":"play sound", "sound": 10, "volume": 70}
    {"set haptic intensity", set_haptic_intensity},                 // {"cmd":"set haptic intensity", "intensity": 50}
    {"update system ip", update_system_ip},                         // {"cmd":"update system ip","ip": "10.12.99.215"}
    {"update system baudrate", update_system_baudrate},             // {"cmd":"update system baudrate","baudrate uart 1": 38400, "baudrate uart 2": 0}
    {"update socket inactive timer", update_socket_inactive_timer}, // {"cmd":"update socket inactive timer","ms": 40000}
    // Add more commands as needed
};

#include <stdbool.h>
#define NUM_BUTTONS 18
// Define the state of each button
bool buttonState[NUM_BUTTONS] = {false};

// Check if a button is turned on or off. After that handle behaviour based on wanted activity.
void getButtonState(char command[]){
    char buttonHex[3] = {command[6], command[7], '\0'}; // Extract button index from hex command
    int value = strtol(buttonHex, NULL, 16); // Convert hex string to integer
    for(int i = 0; i < NUM_BUTTONS;i++){
        if(i == value){
            if(buttonState[i] == true){
                buttonState[i] = false;
                checkDependancy(value, false);
                // printf("%s", command);
                return;
            }else{
                buttonState[i] = true;
                checkDependancy(value, true);
            }
        }
    }
}

// Check if buttons need to be activated based on specific button activity.
void checkDependancy(char buttonIndex, bool state){
    switch (buttonIndex)
    {
    case 5:
        buttonState[1] = state;
        buttonState[2] = state;
        buttonState[3] = state;
        buttonState[4] = state;
        int buttonStates[] = {1,2,3,4};
        playSoundEffect(39,70);
        setButtonActivity(buttonStates,state,10,4);
        break;
    case 11:
        buttonState[12] = state;
        buttonState[13] = state;
        buttonState[14] = state;
        buttonState[15] = state;
        int buttonStates3[] = {12,13,14,15};
        setButtonActivity(buttonStates3,state,10,4);
        break;
    case 16:
        for(int i = 1; i < NUM_BUTTONS; i++){
            buttonState[i] = state;
        }
        int buttonStates2[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
        setButtonActivity(buttonStates2,state,10,16);
        break;
    case 17:
        buttonState[5] = state;
        buttonState[6] = state;
        buttonState[7] = state;
        buttonState[8] = state;
        buttonState[9] = state;
        buttonState[10] = state;
        buttonState[11] = state;
        int buttonStates4[] = {5,6,7,8,9,10,11,16};
        setButtonActivity(buttonStates4,state,10,8);
        break;
    default:
        break;
    }
}

// Play sound effect using the API/JSON. 
void playSoundEffect(int sound, int volume){
    // Create a JSON object and populate it
    //{"cmd":"play sound", "sound": 39, "volume": 70}
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "cmd", "play sound");
    cJSON_AddNumberToObject(root, "sound", sound);
    cJSON_AddNumberToObject(root, "volume", volume);                
    play_sound(root);
    // Free resources
    cJSON_Delete(root);
}

// Set the button activity using the API/JSON. Turn the buttons on/off, fade-in/fade-out. 
void setButtonActivity(int buttons[],int fade, int speed, int buttonAmount){
    // Create a JSON object and populate it
    //{"cmd":"fade button group", "buttons": [1,2,3,4], "fade": 0, "speed": 10}
    cJSON *root = cJSON_CreateObject();
    // Create a cJSON array
    cJSON *buttons_array = cJSON_CreateArray();
    cJSON_AddStringToObject(root, "cmd", "fade button group");

    for(int i = 0; i < buttonAmount; i++){
        cJSON_AddItemToArray(buttons_array, cJSON_CreateNumber(buttons[i]));
    }

    // Add the array to the root object
    cJSON_AddItemToObject(root, "buttons", buttons_array);
    cJSON_AddNumberToObject(root, "fade", fade);
    cJSON_AddNumberToObject(root, "speed", speed);                 
    fade_button_group(root);

    // Free resources
    cJSON_Delete(root);
}

// Initialize api socket 
int32_t init_api_socket(uint8_t sn, uint8_t *buf, uint16_t port)
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

    switch (status) // Check the status of the socket
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
                return ret; // Check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            received_size = (uint16_t)ret;
            sentsize = 0;

            while (received_size != sentsize)
            {
                api_socket_behaviour(buf + sentsize, received_size - sentsize, "cmd");
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
                return ret; // Check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            received_size = (uint16_t)ret;
            sentsize = 0;

            while (received_size != sentsize)
            {
                api_socket_behaviour(buf + sentsize, received_size - sentsize, "cmd");
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
        if (tmp != sn) // Reinitialize the socket 
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

void api_socket_behaviour(uint8_t *buf, datasize_t len, char *item){
    cJSON *json = cJSON_Parse((const char *)buf);
    if (json == NULL) {
        printf("Error parsing JSON: %s\n\n", cJSON_GetErrorPtr());
        // Handle parsing error
    } else {
        // Parse JSON object here
        cJSON *message = cJSON_GetObjectItem(json, item);
        if (message != NULL) {
            //printf("Received item value: %s\n", message->valuestring);
            api_command(message->valuestring, json);
            // Handle the message
        } else {
            printf("Invalid or missing item field in JSON\n\n");
            // Handle missing or invalid field error
        }
        cJSON_Delete(json); // Free cJSON object after use
    }
}

// Function to handle the API command
void api_command(const char *command, cJSON *object) {
    // Check if the command is known
    for (size_t i = 0; i < sizeof(command_handlers) / sizeof(command_handlers[0]); ++i) {
        if (strcmp(command, command_handlers[i].command_name) == 0) {
            // Run found command handler
            command_handlers[i].handler(object);
            return;
        }
    }
    // Command is not known
    handle_unknown_command();
}

// Handler function for an unknown command
void handle_unknown_command() {
    printf("Unknown command\n\n");
}

//{"cmd":"button fade", "fade":9, "speed":50, "button":1}
// Handler function for the button fade command
void button_fade_command(cJSON *object) {
    cJSON *message = cJSON_GetObjectItem(object, "button");
    cJSON *fadeItem = cJSON_GetObjectItem(object, "fade");
    cJSON *speedItem = cJSON_GetObjectItem(object, "speed");

    char command[22]; // Command buffer with space for null terminator
    char buttonHex[5];
    sprintf(buttonHex, "%02X", message->valueint);
    char command1[] = ">$080111420D\x0D\x0A"; // First command
    char command2[] = ">$0801110000\x0D\x0A"; // Second command

    // Extract initial values from the commands
    int value1, value2;
    sscanf(command1, ">$080111%04X\x0D\x0A", &value1);
    sscanf(command2, ">$080111%04X\x0D\x0A", &value2);
    int steps = 10; // Number of steps for fading between the commands

    if(fadeItem->valueint > 0){
        //fade-in with speedItem as speed value
        printf("API fade-in button: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
        // Gradually fade from command1 to command2
        for (int i = 0; i <= steps; i++) {
            // Interpolate values between command1 and command2
            int value_interpolated = value1 + (value2 - value1) * i / steps;
            snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
            sprintf(buttonHex, "%02X", message->valueint);
            command[6] = buttonHex[0];
            command[7] = buttonHex[1];
            hardware_UART_send_data(UART0_ID, command);
            sleep_ms(speedItem->valueint); // Adjust delay time as needed
            if (UART_INTERRUPTED) {
                snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
                sprintf(buttonHex, "%02X", message->valueint);
                command[6] = buttonHex[0];
                command[7] = buttonHex[1];
                hardware_UART_send_data(UART0_ID, command);
                break; // Exit the loop if fading should stop
            }
        }
    }else{
        //fade-out with speedItem as speed value
        printf("API fade-out button: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
        // Gradually fade from command2 back to command1
        for (int i = steps; i >= 0; i--) {
            // Interpolate values between command2 and command1
            int value_interpolated = value1 + (value2 - value1) * i / steps;
            snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
            sprintf(buttonHex, "%02X", message->valueint);
            command[6] = buttonHex[0];
            command[7] = buttonHex[1];
            hardware_UART_send_data(UART0_ID, command);
            sleep_ms(speedItem->valueint); // Adjust delay time as needed
            if (UART_INTERRUPTED) {
                snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
                sprintf(buttonHex, "%02X", message->valueint);
                command[6] = buttonHex[0];
                command[7] = buttonHex[1];
                hardware_UART_send_data(UART0_ID, command);
                break; // Exit the loop if fading should stop
            }
        }
    }
}

// Handler function for commands like:
// {
//     "cmd":"cmd2", 
//     "data":{
//         "id":10, 
//         "name":"test"
//     }
// }
void handle_command2(cJSON *object) {
    cJSON *dataItem = cJSON_GetObjectItem(object, "data");
    // printf("Handling 2nd command type:\n - 1st: %d\n - 2nd: %s\n", dataItem->child->valueint, dataItem->child->next->valuestring);

    if (dataItem != NULL) {
        // Iterate over all the keys in the "data" object
        cJSON *child = dataItem->child;
        while (child != NULL) {
            printf("Key: %s, Value: %s\n", child->string, cJSON_Print(child));
            child = child->next;
        }
        printf("\n");
    }else{
        printf("Invalid or missing data item field in JSON\n\n");
    }
}

// Handler function for commands like:
// {
//     "cmd":"cmd2", 
//     "buttons": [10,2,100]
// }
void handle_command3(cJSON *object) {
    cJSON *dataItem = cJSON_GetObjectItem(object, "buttons");
    // printf("Handling 2nd command type:\n - 1st: %d\n - 2nd: %s\n", dataItem->child->valueint, dataItem->child->next->valuestring);

    if (dataItem != NULL) {
        // Iterate over the array elements
        int array_size = cJSON_GetArraySize(dataItem);
        printf("Array size: %d\n", array_size);
        for (int i = 0; i < array_size; ++i) {
            cJSON *item = cJSON_GetArrayItem(dataItem, i);
            printf("Array element at index %d: %d\n", i, item->valueint);
        }
        printf("\n");
    }else{
        printf("Invalid or missing buttons item field in JSON\n\n");
    }
}

//{"cmd":"fade gui", "fade":9, "speed":50}
// Fade in/out all ui buttons
void fade_gui(cJSON *object){
    cJSON *fadeItem = cJSON_GetObjectItem(object, "fade");
    cJSON *speedItem = cJSON_GetObjectItem(object, "speed");
    char command[22]; // Command buffer with space for null terminator
    char buttonHex[5];

    char command1[] = ">$080111420D\x0D\x0A"; // First command
    char command2[] = ">$0801110000\x0D\x0A"; // Second command

    // Extract initial values from the commands
    int value1, value2;
    sscanf(command1, ">$080111%04X\x0D\x0A", &value1);
    sscanf(command2, ">$080111%04X\x0D\x0A", &value2);
    int steps = 10; // Number of steps for fading between the commands

    if(fadeItem->valueint > 0){
        // Gradually fade from command1 to command2
        for (int i = 0; i <= steps; i++) {
            // Interpolate values between command1 and command2
            int value_interpolated = value1 + (value2 - value1) * i / steps;
            snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
            for(int i = 0; i < 18; i++){
                sprintf(buttonHex, "%02X", i);
                command[6] = buttonHex[0];
                command[7] = buttonHex[1];
                hardware_UART_send_data(UART0_ID, command);
            }
            sleep_ms(speedItem->valueint); // Adjust delay time as needed
            if (UART_INTERRUPTED) {
                snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value2);
                for(int i = 0; i < 18; i++){
                    sprintf(buttonHex, "%02X", i);
                    command[6] = buttonHex[0];
                    command[7] = buttonHex[1];
                    hardware_UART_send_data(UART0_ID, command);
                }
                break; // Exit the loop if fading should stop
            }
        }
    }else{
        //fade-out with speedItem as speed value
        // Gradually fade from command2 back to command1
        for (int i = steps; i >= 0; i--) {
            // Interpolate values between command2 and command1
            int value_interpolated = value1 + (value2 - value1) * i / steps;
            snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
            for(int i = 0; i < 18; i++){
                sprintf(buttonHex, "%02X", i);
                command[6] = buttonHex[0];
                command[7] = buttonHex[1];
                hardware_UART_send_data(UART0_ID, command);
            }
            sleep_ms(speedItem->valueint); // Adjust delay time as needed
            if (UART_INTERRUPTED) {
                snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value1);
                for(int i = 0; i < 18; i++){
                    sprintf(buttonHex, "%02X", i);
                    command[6] = buttonHex[0];
                    command[7] = buttonHex[1];
                    hardware_UART_send_data(UART0_ID, command);
                }
                break; // Exit the loop if fading should stop
            }
        }
    }
        // UART_INTERRUPTED = 0;
}

// {"cmd":"press button", "button": 10}
// Simulate a specific button being pressed
void press_button(cJSON *object){
    char pressStr[] =   "<$061311FF\x0D\x0A";
    char releaseStr[] = "<$06131100\x0D\x0A";
    char buttonHex[5];

    cJSON *message = cJSON_GetObjectItem(object, "button");
    printf("API pressed button: %d\n\n", message->valueint);

    sprintf(buttonHex, "%02X", message->valueint);
    pressStr[6] = buttonHex[0];
    pressStr[7] = buttonHex[1];
    releaseStr[6] = buttonHex[0];
    releaseStr[7] = buttonHex[1];

    send(0, pressStr, 12);
    delay_us(300);
    send(0, releaseStr, 12);
    //>$0801110000\x0D\x0A
    //>$080111420D\x0D\x0A
}

// {"cmd":"fade button group", "buttons": [1,2,10,17], "fade": 1, "speed": 50}
// Fade button leds per group
void fade_button_group(cJSON *object){
    cJSON *message = cJSON_GetObjectItem(object, "buttons");
    cJSON *fadeItem = cJSON_GetObjectItem(object, "fade");
    cJSON *speedItem = cJSON_GetObjectItem(object, "speed");
    int array_size = cJSON_GetArraySize(message);

    char command[22]; // Command buffer with space for null terminator
    char buttonHex[5];
    char command1[] = ">$080111420D\x0D\x0A"; // First command
    char command2[] = ">$0801110000\x0D\x0A"; // Second command

    // Extract initial values from the commands
    int value1, value2;
    sscanf(command1, ">$080111%04X\x0D\x0A", &value1);
    sscanf(command2, ">$080111%04X\x0D\x0A", &value2);
    int steps = 10; // Number of steps for fading between the commands

    if(fadeItem->valueint > 0){
        // printf("API fade-in button: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
        // Gradually fade from command1 to command2
        for (int i = 0; i <= steps; i++) {
            // Interpolate values between command1 and command2
            int value_interpolated = value1 + (value2 - value1) * i / steps;
            snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
            for(int i = 0; i < array_size; i++){
                sprintf(buttonHex, "%02X", cJSON_GetArrayItem(message, i)->valueint);
                command[6] = buttonHex[0];
                command[7] = buttonHex[1];
                hardware_UART_send_data(UART0_ID, command);
            }
            // sleep_ms(speedItem->valueint); // Adjust delay time as needed
            if (UART_INTERRUPTED) {
                snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value2);
                for(int i = 0; i < array_size; i++){
                    sprintf(buttonHex, "%02X", cJSON_GetArrayItem(message, i)->valueint);
                    command[6] = buttonHex[0];
                    command[7] = buttonHex[1];
                    hardware_UART_send_data(UART0_ID, command);
                }
                break; // Exit the loop if fading should stop
            }
        }
    }else{
        // printf("API fade-out button: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
        // Gradually fade from command2 back to command1
        for (int i = steps; i >= 0; i--) {
            // Interpolate values between command2 and command1
            int value_interpolated = value1 + (value2 - value1) * i / steps;
            snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value_interpolated);
            for(int i = 0; i < array_size; i++){
                sprintf(buttonHex, "%02X", cJSON_GetArrayItem(message, i)->valueint);
                command[6] = buttonHex[0];
                command[7] = buttonHex[1];
                hardware_UART_send_data(UART0_ID, command);
            }
            // sleep_ms(speedItem->valueint); // Adjust delay time as needed
            if (UART_INTERRUPTED) {
                snprintf(command, sizeof(command), ">$080111%04X\x0D\x0A", value1);
                for(int i = 0; i < array_size; i++){
                    sprintf(buttonHex, "%02X", cJSON_GetArrayItem(message, i)->valueint);
                    command[6] = buttonHex[0];
                    command[7] = buttonHex[1];
                    hardware_UART_send_data(UART0_ID, command);
                }
                break; // Exit the loop if fading should stop
            }
        }
    }
}

// {"cmd":"set button brightness", "button": 5, "brightness": 20}
// Set birghtness of specific button led
void set_button_brightness(cJSON *object){
    cJSON *message = cJSON_GetObjectItem(object, "button");
    cJSON *brightnessItem = cJSON_GetObjectItem(object, "brightness");
    printf("Button: %d brightness set to: %d\n\n", message->valueint, brightnessItem->valueint);
}

// {"cmd":"set button brightnesses", "buttons": [1,17,3,8], "brightness": 20}
// Set brightness of a group of button leds
void set_button_brightness_group(cJSON *object){
    cJSON *dataItem = cJSON_GetObjectItem(object, "buttons");
    cJSON *brightnessItem = cJSON_GetObjectItem(object, "brightness");

    if (dataItem != NULL) {
        // Iterate over the array elements 
        int array_size = cJSON_GetArraySize(dataItem);
        printf("Array size: %d\n", array_size);
        for (int i = 0; i < array_size; ++i) {
            cJSON *item = cJSON_GetArrayItem(dataItem, i);
            printf("Button: %d brightness set to: %d\n", item->valueint, brightnessItem->valueint);
        }
        printf("\n");
    }else{
        printf("Invalid or missing buttons item field in JSON\n\n");
    }
}

// {"cmd":"restart system"}
// Restart system
void system_restart(cJSON *object){
    printf("API restarting system \n\n");
    sleep_ms(2000);
    watchdog_enable(1, 1);
}

// {"cmd":"disconnect socket", "socket": 5}
// Disconnect socket with a specific id
void disconnect_socket(cJSON *object){
    cJSON *socketItem = cJSON_GetObjectItem(object, "socket");
    printf("API disconnected socket: %d\n\n", socketItem->valueint);
    disconnect(socketItem->valueint);
}

// {"cmd":"calibrate buttons"}
// Calibrate buttons (sensitivity etc)
void calibrate_buttons(cJSON *object){
    printf("API calibrating buttons\n\n");
}

// {"cmd":"set button color", "button": 5, "color": "green"}
// Set color of specific button led
void set_button_color(cJSON *object){
    cJSON *buttonItem = cJSON_GetObjectItem(object, "button");
    cJSON *colorItem = cJSON_GetObjectItem(object, "color");
    printf("API gave button: %d the color: %s\n\n", buttonItem->valueint, colorItem->valuestring);
}

// {"cmd":"set button colors", "buttons": [1,10,7,13], "color": "green"}
// Set color of a group of button leds
void set_button_color_group(cJSON *object){
    cJSON *dataItem = cJSON_GetObjectItem(object, "buttons");
    cJSON *colorItem = cJSON_GetObjectItem(object, "color");

    if (dataItem != NULL) {
        // Iterate over the array elements
        int array_size = cJSON_GetArraySize(dataItem);
        printf("Array size: %d\n", array_size);
        for (int i = 0; i < array_size; ++i) {
            cJSON *item = cJSON_GetArrayItem(dataItem, i);
            printf("API gave button: %d the color: %s\n", item->valueint, colorItem->valuestring);
        }
        printf("\n");
    }else{
        printf("Invalid or missing buttons item field in JSON\n\n");
    }
}

// {"cmd":"play sound", "sound": 10, "volume": 70}
// Play a specific sound
void play_sound(cJSON *object){
    cJSON *soundItem = cJSON_GetObjectItem(object, "sound");
    cJSON *volumeItem = cJSON_GetObjectItem(object, "volume");
    // printf("API plating sound: %d with volume: %d\n\n", soundItem->valueint, volumeItem->valueint);

    char soundHex[3];
    snprintf(soundHex, sizeof(soundHex), "%02X", soundItem->valueint);

    uint8_t txbuf[10];
    txbuf[0] = 0xF0;
    txbuf[1] = 0xAA;
    txbuf[2] = 0x0A;
    txbuf[3] = 0x03;
    txbuf[4] = 0x01;
    sscanf(soundHex, "%hhx", &txbuf[5]);
    txbuf[6] = 0x00;
    txbuf[7] = 0x00;
    txbuf[8] = 0x01;
    txbuf[9] = 0x55;
    // \xF0\xAA\x0A\x03\x01\x27\x00\x00\x01\x55
    for(int i = 0; i < 10; i++){
        uart_transmit_command(txbuf[i]);
        sleep_ms(5);
        // printf("0x%02X", txbuf[i]);
    }
    //printf("\n");
}

// {"cmd":"set haptic intensity", "intensity": 50}
// Set the intensity of the haptic feedback
void set_haptic_intensity(cJSON *object){
    cJSON *intensityItem = cJSON_GetObjectItem(object, "intensity");
    printf("API set haptic intensity to: %d\n\n", intensityItem->valueint);
}

// {"cmd":"update system ip","ip": "10.12.99.215"}
// Update the network ip and initialize a new network
void update_system_ip(cJSON *object){
    cJSON *ipItem = cJSON_GetObjectItem(object, "ip");
    int a, b, c, d;
    sscanf(ipItem->valuestring, "%d.%d.%d.%d", &a, &b, &c, &d);

    g_net_info.ip[0] = a;
    g_net_info.ip[1] = b;
    g_net_info.ip[2] = c;
    g_net_info.ip[3] = d;
    ipTemplate[0] = a;
    ipTemplate[1] = b;
    ipTemplate[2] = c;
    ipTemplate[3] = d;
    close(0);
    close(1);
    close(2);
    close(3);
    close(5);
    network_initialize(g_net_info);
    print_network_information(g_net_info);
    printf("API updated ip to: %s\n\n", ipItem->valuestring);
}

// {"cmd":"update system baudrate","baudrate uart 1": 38400, "baudrate uart 2": 0}
// Update baudrate for specific uart (put 0 if the baudrate does not need to be updated)
void update_system_baudrate(cJSON *object){
    cJSON *baudrateItem1 = cJSON_GetObjectItem(object, "baudrate uart 1");
    cJSON *baudrateItem2 = cJSON_GetObjectItem(object, "baudrate uart 2");
    if(baudrateItem1->valueint > 0){
        uart_set_baudrate(UART0_ID, baudrateItem1->valueint);
        printf("API updated uart 1 baudrate to: %d\n\n", baudrateItem1->valueint);
    }

    if(baudrateItem2->valueint > 0){
        uart_set_baudrate(UART1_ID, baudrateItem2->valueint);
        printf("API updated uart 2 baudrate to: %d\n\n", baudrateItem2->valueint);
    }
}

// {"cmd":"update socket inactive timer","ms": 40000}
// Update the timer that handles disconnecting an inactive socket
void update_socket_inactive_timer(cJSON *object){
    cJSON *timerItem = cJSON_GetObjectItem(object, "sec");
    socketInactiveTimer = timerItem->valueint;
    printf("API updated socket inactive disconnect timer to: %d\n\n", timerItem->valueint);
}