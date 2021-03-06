#include "broker_support.h"
#include "common.h"

struct ClientRequestRegistration *reg_clnt_req(char* op,
		Location* src, int message_id, struct Config *brokerConfig) {

	if (brokerConfig->verbose) {
		PRINT("Registering client service request, '%s:%s'(%s)\n", src->address,
				src->port, op);
	}

	ClientReg* client_request_registration;

	client_request_registration = malloc(
			sizeof(struct ClientRequestRegistration));
	client_request_registration->address = src->address;
	client_request_registration->port = src->port;
	client_request_registration->operation = op;
	client_request_registration->message_id = message_id;

	LIST_Add(brokerConfig->clnt_req_repo, client_request_registration);

	return client_request_registration;
}
