#include "server_interface.h"
#include <string.h>
#include <stulibc.h>


char* services[] = {"getBrokerName","echo","add","sayHello","getServerDate",NULL};
char* getServerDate() 
{
    return  "20 jan 2012";
}

char* getBrokerName()
{
    char* name = Alloc( sizeof(char) );
    strcpy( name, "broker v0.1");
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

char* sayHello(int age, char* name)
{
	char* buffer = MEM_Alloc(sizeof(char) * 80 );
	if( buffer != null ){
		snprintf(buffer, 80, "Hello %s, you are %d years old", name, age);
	} else {
		return "failed.";
	}

	return buffer;
}
