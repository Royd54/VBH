#include "socket.h"
#include "../libraries/cJSON/cJSON.h"
#include "tcp_server.c"

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
    
    if(fadeItem->valueint > 0){
        //fade-in with speedItem as speed value
        printf("API fade-in button: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
    }else{
        //fade-out with speedItem as speed value
        printf("API fade-out button: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
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
    cJSON *message = cJSON_GetObjectItem(object, "fade");
    cJSON *speedItem = cJSON_GetObjectItem(object, "speed");

    if(message->valueint > 0){
        //fade-in with speedItem as speed value
        printf("Fading-in gui with value: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
    }else{
        //fade-out with speedItem as speed value
        printf("Fading-out gui with value: %d and speed: %d\n\n", message->valueint, speedItem->valueint);
    }
}

// {"cmd":"press button", "button": 10}
// Simulate a specific button being pressed
void press_button(cJSON *object){
    cJSON *message = cJSON_GetObjectItem(object, "button");
    printf("API pressed button: %d\n\n", message->valueint);
}

// {"cmd":"fade button group", "buttons": [1,2,10,17], "fade": 1, "speed": 50}
// Fade button leds per group
void fade_button_group(cJSON *object){
    cJSON *dataItem = cJSON_GetObjectItem(object, "buttons");
    cJSON *message = cJSON_GetObjectItem(object, "fade");
    cJSON *speedItem = cJSON_GetObjectItem(object, "speed");
    // printf("Handling 2nd command type:\n - 1st: %d\n - 2nd: %s\n", dataItem->child->valueint, dataItem->child->next->valuestring);

    if (dataItem != NULL) {
        // Iterate over the array elements
        int array_size = cJSON_GetArraySize(dataItem);
        printf("Array size: %d\n", array_size);
        for (int i = 0; i < array_size; ++i) {
            cJSON *item = cJSON_GetArrayItem(dataItem, i);
            if(message->valueint > 0){
                //fade-in with speedItem as speed value
                printf("Fading-in button: %d with speed: %d\n", item->valueint, speedItem->valueint);
            }else{
                //fade-out with speedItem as speed value
                printf("Fading-out button: %d with speed: %d\n", item->valueint, speedItem->valueint);
            }
        }
        printf("\n");
    }else{
        printf("Invalid or missing buttons item field in JSON\n\n");
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
    printf("API plating sound: %d with volume: %d\n\n", soundItem->valueint, volumeItem->valueint);
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