#include "common.h"
#include "broker_support.h"

extern char port[MAX_PORT_CHARS];
extern struct ServiceRegistration service_repository;

void forward_response(Packet packet, struct sockaddr_in* peerp)
{
    Destination* dest = find_client_for_response(packet);
    send_request( packet, dest->address, dest->port, false );
}
