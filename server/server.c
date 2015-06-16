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
    PRINT("Calling getBrokerName()\n");
    char* name = Alloc( sizeof(char) );
    strcpy( name, "peter");
    return name;
}

void echo(char* data)
{
    PRINT("Calling echo(char* data)\n");
    STR_Reverse(data);
    PRINT("%s",data);
}

int add( int one, int two )
{
    return (one + two);
}


