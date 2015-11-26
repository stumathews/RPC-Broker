#include "common.h"
#include "broker_support.h"

void forward_response(Packet* response)
{
	Location* client = Alloc(sizeof(Location));
    find_client_for_response(response, client);
    send_request( response, client->address, client->port, false );
    MEM_DeAlloc(client, "client");
}
