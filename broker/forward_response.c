#include "common.h"
#include "broker_support.h"

extern char port[MAX_PORT_CHARS];
extern struct ServiceRegistration service_repository;

void forward_response(char* buffer, int len, struct sockaddr_in* peerp)
{
    Destination *dest = Alloc( sizeof( Destination ));
    dest = find_client(buffer, len);
    send_request( buffer, len, dest->address, dest->port, false );

    //list_del( &crreg->list);

}
