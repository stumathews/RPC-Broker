#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

extern char server_address[MAX_ADDRESS_CHARS];
extern char port[MAX_PORT_CHARS];

/**
 * @brief Craft a service registration message and send it of fto the broker.
 * 
 * @param broker_address the broker address
 * @param broker_port the broker port
 * @return void
 */
void service_register_with_broker(Details brokerDetails, Details serverDetails,
		Config brokerConfig) {
	ServiceReg *sr = malloc(sizeof(ServiceReg));
	sr->address = serverDetails.address; // TODO: Get our actual IP address
	sr->port = serverDetails.port;

	msgpack_sbuffer sbuf;
	msgpack_sbuffer_init(&sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	pack_map_int(REQUEST_TYPE_HDR, SERVICE_REGISTRATION, &pk);
	pack_map_str(SENDER_ADDRESS_HDR, sr->address, &pk);
	pack_map_str(REPLY_PORT_HDR, sr->port, &pk);
	pack_map_str(SERVICE_NAME_HDR, "theServiceName", &pk);

	// pull in the services defined in the server code: server.c
	extern char* services[];
	char* service = services[0];
	int i = 0;
	while (services[i] != NULL) {
		if (brokerConfig.verbose) {
			PRINT("Service %s.\n", services[i]);
		}
		i++;
	}
	pack_map_int(SERVICES_COUNT_HDR, i, &pk);
	msgpack_pack_map(&pk, 1);
	msgpack_pack_str(&pk, 8);
	msgpack_pack_str_body(&pk, SERVICES_HDR, 8);
	msgpack_pack_array(&pk, i);

	if (brokerConfig.verbose) {
		PRINT("num services %d\n", i);
	}

	// pack the services into the protocol message
	while (i >= 0) {
		if (!STR_IsNullOrEmpty(services[i])) {
			if (brokerConfig.verbose) {
				PRINT("service packed is %s\n", services[i]);
			}
			msgpack_pack_str(&pk, strlen(services[i]));
			msgpack_pack_str_body(&pk, services[i], strlen(services[i]));
		}
		i--;
	}
	// send registration message to broker
	Packet pkt;
	pkt.buffer = sbuf.data;
	pkt.len = sbuf.size;
	send_request(&pkt, brokerDetails.address, brokerDetails.port,
			brokerConfig.verbose);
	msgpack_sbuffer_destroy(&sbuf);
}
