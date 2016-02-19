#include "common.h"
#include "broker_support.h"
#include <unistd.h>

void forward_response_to_client(Packet* response)
{
    Location* client = malloc(sizeof(Location));

    find_client_for_response(response, client);
    send_request(response, client->address, client->port, false);

    free(client->port);
    free(client->address);
    free(client);

}
