#include "server_interface.h" // use the server API
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stulibc.h>

void call_server()
{
}

void start_task()
{
}

void use_Broker_API()
{
}
char broker_address[30] = {0};
char broker_port[20] = {0};

static void setBrokerPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( broker_port, arg, strlen(arg));
}

static void setBrokerAddress(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( broker_address, arg, strlen(arg));

}

int main( int argc, char* argv[])
{
    LIB_Init();

    struct Argument* portNumber = CMD_CreateNewArgument("port",
                                                        "port <number>",
                                                        "Set the broker port that the client will connect to",
                                                        true,
                                                        true,
                                                        setBrokerPortNumber);
    struct Argument* brokerAddress = CMD_CreateNewArgument("address",
                                                        "address <address>",
                                                        "Set the broker address that the client will connect to",
                                                        true,
                                                        true,
                                                        setBrokerAddress);
    CMD_AddArgument(portNumber);
    CMD_AddArgument(brokerAddress);

    if( argc > 1 )
    {
        enum ParseResult result = CMD_Parse(argc,argv,true);
        if( result != PARSE_SUCCESS )
        {
            return 1;
        }
    }
    else
    {
        CMD_ShowUsages("client <options>");
        exit(0);
    }

    INIT();
    char* strServerDate = (char*) calloc( 256,sizeof(char) ); // 256 byte buffer
    memset( strServerDate, '\0', 256);
    
    // use the server API...implemented by the client-proxy's getServerDate();
    getServerDate(strServerDate,256);
    printf("Returned result from server was: %s\n",strServerDate);

    LIB_Uninit();
    EXIT(0);
}
