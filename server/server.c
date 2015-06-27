#include "server_interface.h"
#include <string.h>
#include <stulibc.h>


char* services[] = {"getServerDate","getBrokerName","echo","add",NULL};

char* getServerDate() 
{
    return  "20 jan 2012";
}

char* getBrokerName()
{
    char* name = Alloc( sizeof(char) );
    strcpy( name, "peter");
    return name;
}

char* echo(char* data)
{
    STR_Reverse(data);
    return data;
}

int add( int one, int two )
{
    return (one + two);
}


