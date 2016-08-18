#include "common.h"
#include "broker_support.h"
#include <unistd.h>

void fwd_response_to_clnt(Packet* response, struct Config *brokerConfig)
{
	Location* client = malloc(sizeof(Location));
	find_client_for_response(response, client, brokerConfig);
	send_req(response, client->address, client->port, false);
}
