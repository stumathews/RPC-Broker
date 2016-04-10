#include "broker_support.h"
#include "common.h"

extern List service_repository;
static void perform_diagnostics(
		struct ServiceRegistration* service_registration, bool verbose_flag);

void register_service_request(Packet* packet, struct Config *brokerConfig) {
	struct ServiceRegistration *service_registration;
	service_registration = unpack_service_registration_buffer(packet->buffer,
			packet->len, brokerConfig);

	LIST_Add(&service_repository, service_registration);

	if (brokerConfig->verbose) {
		PRINT(">> SERVICE_REGISTRATION ACK\n");
	}
	msgpack_sbuffer sbuf;
	msgpack_sbuffer_init(&sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	int message_id = get_header_int_value(packet, MESSAGE_ID_HDR);

	pack_map_int(REQUEST_TYPE_HDR, SERVICE_REGISTRATION_ACK, &pk);
	pack_map_int(MESSAGE_ID_HDR, message_id, &pk);

	/*
	 {"request-type"=>2}
	 {"sender-address"=>"127.0.0.1"}
	 {"reply-port"=>"8181"}
	 {"service-name"=>"theServiceName"}
	 
	 */
	char* reply_port = get_header_str_value(packet, REPLY_PORT_HDR);
	char* sender_address = get_header_str_value(packet, SENDER_ADDRESS_HDR);
	packet->buffer = sbuf.data;
	packet->len = sbuf.size;
	sleep(5);
	send_request(packet, sender_address, reply_port, brokerConfig->verbose);

	msgpack_sbuffer_destroy(&sbuf);

	if (brokerConfig->verbose) {
		perform_diagnostics(service_registration, brokerConfig->verbose);
	}
}

static void perform_diagnostics(
		struct ServiceRegistration* service_registration, bool verbose_flag) {
	/*
	 PRINT("Registering service '%s' from host '%s':\n",service_registration->service_name, service_registration->address);
	 for(int i = 0 ; i < service_registration->num_services;i++) {
	 PRINT("Registering operation '%s'\n", service_registration->services[i]);
	 }
	 */
	print_service_repository();
}
