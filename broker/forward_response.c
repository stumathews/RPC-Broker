
#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern struct ServiceRegistration service_repository;

// When the broker gets a response form the server, it will need to send it back to the originting client that requeted it.
void forward_response(char* buffer, int len)
{
    Destination *dest = Alloc( sizeof( Destination ));
    dest->address = "SAMPLE";
    dest->port = "2222";
    find_client(buffer, len, dest); // the socket is already connected to the client if this is synchrnous

}
