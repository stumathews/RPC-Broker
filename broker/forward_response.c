#include "common.h"
#include "broker_support.h"

extern char port[MAX_PORT_CHARS];
extern struct ServiceRegistration service_repository;

void forward_response(char* buffer, int len)
{
    Destination *dest = Alloc( sizeof( Destination ));
    dest->address = "SAMPLE";
    dest->port = "2222";
    find_client(buffer, len, dest);

}
