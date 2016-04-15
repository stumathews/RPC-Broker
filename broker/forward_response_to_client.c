#include "common.h"
#include "broker_support.h"
#include <unistd.h>

void forward_response_to_client(Packet* response, struct Config *brokerConfig)
{
	Location* client = malloc(sizeof(Location));
	find_client_for_response(response, client, brokerConfig);
	PRINT(">>> Forward response to client\n");
	send_request(response, client->address, client->port, false);
}
