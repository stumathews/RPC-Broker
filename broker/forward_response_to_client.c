#include "common.h"
#include "broker_support.h"
#include <unistd.h>

void forward_response_to_client(Packet* response)
{
    List* mem_pool = LIST_GetInstance();
    Location* client = Alloc(sizeof(Location), mem_pool);
    find_client_for_response(response, client);
	
    // Currently we went the response back so quickly that the client hasn't get started listening 
    // Ideally we shouldn't sleep, the client should have a listenting port on a thread which is always ready

    // That being said we should have a retry strategy anyway
    sleep(1);

    send_request(response, client->address, client->port, false);
    MEM_DeAllocAll(mem_pool);
}
