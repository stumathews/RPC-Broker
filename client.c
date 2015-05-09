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

int main( int argc, char* argv[])
{
    INIT();
    char* strServerDate = (char*) calloc( 256,sizeof(char) ); // 256 byte buffer
    memset( strServerDate, '\0', 256);
    
    // use the server API...implemented by the client-proxy's getServerDate();
    getServerDate(strServerDate,256);
    printf("Returned result from server was: %s\n",strServerDate);
}
