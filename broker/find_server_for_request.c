#include "broker_support.h"
#include "common.h"

extern List service_repository;

Location* find_server_for_req(Packet* packet, Config* clientConfig) {
	ServiceReg *sreg_entry;
	struct list_head *pos, *q;
	char* op_name;
	Location *dest;

	op_name = get_op_name(packet);
	dest = malloc(sizeof(Location));
	dest->address = NULL;
	dest->port = NULL;

	if (clientConfig->svc_repo->size == 0) {
		PRINT("No services registered in broker.\n");
		return dest;;
	}
	for (int j = 0; j < clientConfig->svc_repo->size; j++) {
		sreg_entry = (ServiceReg *) LIST_Get(clientConfig->svc_repo, j)->data;

		for (int i = 0; i < sreg_entry->num_services; i++) {
			if (clientConfig->verbose) {
				PRINT("service each in %s\n", op_name);
				PRINT("compare with %s\n", sreg_entry->services[i]);
			}
			if (STR_Equals(op_name, sreg_entry->services[i])) {
				if (clientConfig->verbose)
					PRINT("in found\n");
				dest->address = sreg_entry->address;
				dest->port = sreg_entry->port;
				DBG(
						"FOUND server for required service '%s' at location '%s:%s'\n",
						op_name, dest->address, dest->port);
				return dest;
			}
		}
	}
	free(op_name);
	return dest;
}

