#include "common.h"
#include "broker_support.h"

void forward_response(Packet* response)
{
    Location* client = find_client_for_response(response);
    send_request( response, client->address, client->port, false );
}
