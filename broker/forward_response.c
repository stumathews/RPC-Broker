#include "common.h"
#include "broker_support.h"
#include <unistd.h>

void forward_response(Packet* response)
{
    Location* client = Alloc(sizeof(Location));
    find_client_for_response(response, client);
	
    // Currently we went the response back so quickly that the client hasn't get started listening 
    // Ideally we shouldn't sleep, the client should have a listenting port on a thread which is always ready

    // That being said we should have a retry strategy anyway
    PRINT("Sleeping for 1 second to help client get read for response...\n");
    sleep(1);

    send_request( response, client->address, client->port, false );
    MEM_DeAlloc(client, "client");
}
