#include "socket.h"
#include "../libraries/cJSON/cJSON.h"

#define DATA_BUF_SIZE 2048 //Max size of the data buffer

void api_socket_behaviour(uint8_t *buf, datasize_t len, char *item);
void api_command(const char *command, cJSON *object);
void handle_unknown_command();
void button_fade_command(cJSON *object);
void handle_command2(cJSON *object);

// Structure to represent a command and its handler function
typedef struct {
    const char *command_name;
    void (*handler)();
} CommandHandler;

// Array of command handlers
CommandHandler command_handlers[] = {
    {"button fade", button_fade_command},
    {"cmd2", handle_command2},
    // Add more commands as needed
};

//Initialize api socket 
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
                api_socket_behaviour(buf + sentsize, received_size - sentsize, "cmd"); //Run debugging behaviour
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
                api_socket_behaviour(buf + sentsize, received_size - sentsize, "function"); //Run debugging behaviour
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

void api_socket_behaviour(uint8_t *buf, datasize_t len, char *item){
    cJSON *json = cJSON_Parse((const char *)buf);
    if (json == NULL) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        // Handle parsing error
    } else {
        // Parse JSON object here
        // Example: Assuming JSON object has a field named "message"
        cJSON *message = cJSON_GetObjectItem(json, item);
        if (message != NULL) {
            printf("Received item value: %s\n", message->valuestring);
            api_command(message->valuestring, json);
            // Handle the message
        } else {
            printf("Invalid or missing item field in JSON\n");
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
    printf("Unknown command\n");
}

// Handler function for the button fade command
void button_fade_command(cJSON *object) {
    cJSON *message = cJSON_GetObjectItem(object, "button");
    printf("Handling button fade command for button id: %d\n", message->valueint);
}

// Handler function for command
void handle_command2(cJSON *object) {
    printf("Handling command2\n");
}