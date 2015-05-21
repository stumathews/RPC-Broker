#include "server_interface.h"
#include <string.h>
#include <stulibc.h>

char* services[] = {"getServerDate","getBrokerName","echo",NULL};

void getServerDate( char* buffer, int length )
{
    char* date = "20 jan 2012";
    strncpy( buffer, date, strlen(date) <= length ? strlen(date): length);
}

char* getBrokerName()
{
    char* name = Alloc( sizeof(char) );
    strcpy( name, "peter");
    return name;
}

void echo(char* data)
{
    STR_Reverse(data);
}


