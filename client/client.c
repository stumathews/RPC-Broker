#include "server_interface.h" // use the server API
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stulibc.h>
#include "common.h"

char broker_address[MAX_ADDRESS_CHARS] = {0};
char broker_port[MAX_PORT_CHARS] = {0};
char wait_response_port[MAX_PORT_CHARS] = {0};
bool wait_response_indef = false;
bool verbose = false;


static void setWaitResponsePort(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( wait_response_port, arg, strlen(arg));
}

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

static void setWaitResponseIndef( char* arg)
{
    wait_response_indef = true;
}

static void setVerbose(char* arg)
{
    verbose = true;
}
void setupCmd(int argc, char* argv[])
{

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
    struct Argument* verboseArg = CMD_CreateNewArgument("verbose",
                                                        "",
                                                        "Prints all messages verbosly",
                                                        false,
                                                        false,
                                                        setVerbose);
    
    struct Argument* waitResponsePortArg = CMD_CreateNewArgument("wait-port",
                                                        "",
                                                        "The port that the broker can connect to deliver the response",
                                                        true,
                                                        true,
                                                        setWaitResponsePort);
    struct Argument* waitResponseIndefArg = CMD_CreateNewArgument("wait-reponse-indef",
                                                        "",
                                                        "Should we wait indefinitely for the reponse",
                                                        false,
                                                        false,
                                                        setWaitResponseIndef);
    CMD_AddArgument(portNumber);
    CMD_AddArgument(brokerAddress);
    CMD_AddArgument(verboseArg);
    CMD_AddArgument(waitResponsePortArg);
    CMD_AddArgument(waitResponseIndefArg);

    if( argc > 1 )
    {
        enum ParseResult result = CMD_Parse(argc,argv,true);
        if( result != PARSE_SUCCESS )
        {
            exit(1);
        }
    }
    else
    {
        CMD_ShowUsages("client <options>");
        exit(0);
    }
}

int main( int argc, char* argv[])
{
    LIB_Init();
    
    setupCmd(argc, argv);

    PRINT("Got server date reply as %s", getServerDate());
    PRINT( "reverse echo = %s", echo("Bruce Mathews") );
    PRINT( "broker name = %s", getBrokerName());
    PRINT("sum = %d",add(20,199));

    LIB_Uninit();
}

void call_server()
{
}

void start_task()
{
}

void use_Broker_API()
{
}
