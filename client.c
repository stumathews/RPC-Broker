#include "server_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main( int argc, char* argv[])
{
    char* strServerDate = (char*) calloc( 256,sizeof(char) ); // 256 byte buffer
    memset( strServerDate, '\0', 256);
    
    // Call the client-proxy's getServerDate();
    getServerDate(strServerDate,256);
    printf("Returned result from server was: %s\n",strServerDate);
}
