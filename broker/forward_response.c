#include "common.h"
#include "broker_support.h"

extern char port[MAX_PORT_CHARS];
extern struct ServiceRegistration service_repository;

void forward_response(Packet* response)
{
    Destination* client = find_client_for_response(response);
    send_request( response, client->address, client->port, false );
}
