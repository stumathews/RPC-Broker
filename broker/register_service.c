#include "broker_support.h"
#include "common.h"


static void perform_diagnostics(struct ServiceRegistration* service_registration, struct Config *brokerConfig);

void send_svc_req_ack(struct Config* config, Packet* packet)
{
	int total_sent_bytes = 0;
	int tries = 1;
	char* reply_port = NULL;
	char* sender_address = NULL;

	if (config->verbose) {
		PRINT(">> SERVICE_REGISTRATION ACK\n");
	}

	msgpack_sbuffer sbuf;
	msgpack_sbuffer_init(&sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
	int message_id = get_hdr_int(packet, MESSAGE_ID_HDR);
	pack_map_int(REQUEST_TYPE_HDR, SERVICE_REGISTRATION_ACK, &pk);
	pack_map_int(MESSAGE_ID_HDR, message_id, &pk);
	reply_port = get_hdr_str(packet, REPLY_PORT_HDR);
	sender_address = get_hdr_str(packet, SENDER_ADDRESS_HDR);
	packet->buffer = sbuf.data;
	packet->len = sbuf.size;
	PRINT(">>> SERVICE_REGISTRATION_ACK\n");
	do {
		total_sent_bytes = send_req(packet, sender_address, reply_port,
				config->verbose);
		if (total_sent_bytes == 0) {
			sleep(1);
		}
		tries++;
		if (tries == 3 && total_sent_bytes == 0)
			PRINT(
					"Tried 3 times to ACK register with server and have gaven up - the server went down?\n");
	} while (total_sent_bytes == 0 && tries <= 3);
	msgpack_sbuffer_destroy(&sbuf);
}

void reg_svc_req(Packet* packet, struct Config *config)
{
	struct ServiceRegistration *reg = unpack_service_registration_buffer(packet->buffer, packet->len, config);

	if (config->verbose) {
		PRINT("<< SERVICE_REGISTRATION\n");
	}

	LIST_Add(config->svc_repo, reg);

	send_svc_req_ack(config, packet);

	if (config->verbose) {
		perform_diagnostics(reg, config);
	}
}

static void perform_diagnostics(struct ServiceRegistration* service_registration, struct Config *brokerConfig)
{
	/*
	 PRINT("Registering service '%s' from host '%s':\n",service_registration->service_name, service_registration->address);
	 for(int i = 0 ; i < service_registration->num_services;i++) {
	 PRINT("Registering operation '%s'\n", service_registration->services[i]);
	 }
	 */
	print_service_repository(brokerConfig);
}
