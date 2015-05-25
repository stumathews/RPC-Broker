/**
Server API
*/
#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

struct ServFunction
{
    void* (*function)(void*);
    char name[20];
};

void getServerDate( char* buffer, int length );
void echo(char* echo);
char* getBrokerName();
inline char* Reverse(char* data){}

#endif
