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
