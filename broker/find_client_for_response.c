#include "broker_support.h"
#include "common.h"

Location* find_client_for_response(Packet *packet, Location* dest, 	struct Config *brokerConfig) {
	struct list_head *pos, *q;
	char* requested_operation;
	int* message_id;
	ClientReg *crreg_entry;

	if (brokerConfig->client_request_repository->size == 0) {
		PRINT("No client requests registered in broker\n");
	}

	requested_operation = malloc(sizeof(char));
	message_id = malloc(sizeof(int));

	for (int j = 0; j < brokerConfig->client_request_repository->size; j++) {
		Node* node = LIST_Get(brokerConfig->client_request_repository, j);
		crreg_entry = node->data;
		*message_id = get_header_int_value(packet, MESSAGE_ID_HDR);
		requested_operation = get_header_str_value(packet, OPERATION_HDR);
		if (*message_id == crreg_entry->message_id
				&& STR_Equals(requested_operation, crreg_entry->operation)) {
			dest->address = crreg_entry->address;
			dest->port = crreg_entry->port;
			if (brokerConfig->verbose) {
				DBG("found client at %s:%s\n", dest->address, dest->port);
			}
			LIST_DeleteNode(brokerConfig->client_request_repository, node);
			return dest;
		}
	}
	free(requested_operation);
	free(message_id);
	return dest;
}
