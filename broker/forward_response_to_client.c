#include "common.h"
#include "broker_support.h"
#include <unistd.h>

void forward_response_to_client(Packet* response)
{
    List* mem_pool = LIST_GetInstance();
    Location* client = Alloc(sizeof(Location), mem_pool);

    find_client_for_response(response, client);
    send_request(response, client->address, client->port, false);

    MEM_DeAllocAll(mem_pool);
}
