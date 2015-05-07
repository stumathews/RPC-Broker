#include "server_interface.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// called from the client and realised from server_inerface.h
void getServerDate(char* buffer, int length)
{
    struct broker_message message;
    
    strcpy(message.functionname,"strServerDate");
    
    struct param param1;
        strcpy( param1.name,"buffer");
        strcpy( param1.type, "char*");
        strncpy( param1.value, buffer, length);
    struct param param2;
        strcpy( param2.name, "length");
        strcpy( param2.type, "int");
        sprintf(param2.value, "%d", length );
        message.params[0] = param1;
        message.params[1] = param2;
    printf("sizeof broker_message is %d\n"\
           "sizeof functionname %d\n"\
           "sizeof params %d\n"\
           "sizeof result %d\n", sizeof(message),sizeof(message.functionname),sizeof(message.params), sizeof(message.result));
    // send object to broker and wait for reply
    //struct pack result = send_msg_to_broker( message, sizeof(message) );
    // unpack
    return (char) message.result.result;
}

