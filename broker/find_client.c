#include "broker_support.h"
#include "common.h"

extern bool verbose_flag;

void find_client(char *buffer, int len, Destination *dest)
{
    PRINT("Response for client %s %s\n", dest->address, dest->port);
    unpack_data( buffer,len, verbose_flag);
}
