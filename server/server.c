#include "server_interface.h"
#include <string.h>
#include <stulibc.h>


char* services[] = {"getBrokerName","echo","add","sayHello","getServerDate","sayDog", "diffirence",NULL};
char* getServerDate() 
{
    return  "20 jan 2012";
}

char* getBrokerName()
{ 
    List* mem_pool = LIST_GetInstance();
    char* name = Alloc( sizeof(char), mem_pool );
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
    	List* mem_pool = LIST_GetInstance();
	char* buffer = MEM_Alloc(sizeof(char) * 80, mem_pool );
	if( buffer != null ){
		snprintf(buffer, 80, "Hello %s, you are %d years old", name, age);
	} else {
		return "failed.";
	}

	return buffer;
}

char* sayDog(char* one, char* two, char* three, char* four)
{
	char* first_part = STR_Join(one, two);
	char* second_part = STR_Join(three, four);
	char* final_part = STR_Join(first_part, second_part);

	return final_part;
}

int diffirence(int one, int two)
{
	return two - one;
}
