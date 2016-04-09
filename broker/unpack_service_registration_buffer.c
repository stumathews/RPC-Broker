#include "broker_support.h"
#include "common.h"

struct ServiceRegistration* unpack_service_registration_buffer(
		const char* payload, int langth, struct Config* brokerConfig) {
	PRINT("Unpacking service registration request...\n");

	size_t off = 0;
	int i = 0;
	msgpack_unpacked unpacked_result;
	msgpack_unpack_return return_status;

	struct ServiceRegistration* unpacked = malloc(
			sizeof(struct ServiceRegistration));
	unpacked->num_services = 0;

	msgpack_unpacked_init(&unpacked_result);

	while ((return_status = msgpack_unpack_next(&unpacked_result, payload,
			langth, &off)) == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj;
		msgpack_object val;
		char header_name[MAX_HEADER_NAME_SIZE];
		memset(header_name, '\0', MAX_HEADER_NAME_SIZE);

		obj = unpacked_result.data;
		val = extract_header(&obj, header_name);

		if (val.type == MSGPACK_OBJECT_STR) {
			int str_len;
			char* str;

			str_len = val.via.str.size;
			str = (char*) malloc(sizeof(char) * (size_t) str_len);

			copyString(str_len, &val.via.str, str);

			if (STR_Equals(SENDER_ADDRESS_HDR, header_name) == true) {
				unpacked->address = str;
			} else if (STR_Equals(REPLY_PORT_HDR, header_name) == true) {
				unpacked->port = str;
			} else if (STR_Equals(SERVICE_NAME_HDR, header_name) == true) {
				unpacked->service_name = str;
			}
		} else if (val.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
			if (STR_Equals(SERVICES_COUNT_HDR, header_name) == true) {
				unpacked->num_services = val.via.i64;
				unpacked->services = malloc(
						(sizeof(char*) * (val.via.i64) + val.via.i64));
			}
		} else if (val.type == MSGPACK_OBJECT_ARRAY) {
			PRINT("Processing services %d services...\n", val.via.array.size);

			msgpack_object_array array = val.via.array;

			for (int i = 0; i < array.size; i++) {
				struct msgpack_object curr;
				int str_len;
				char* str;

				curr = array.ptr[i];
				str_len = curr.via.str.size;
				str = (char*) malloc((size_t) str_len + 1);

				if (!str) {
					PRINT("failed to malloc!\n");
					PRINT("curr.via.str.size %u \n", (int )curr.via.str.size);
					perror("");
					return 0;
				}

				copyString(str_len, &curr.via.str, str);
				unpacked->services[i] = str;

				if (brokerConfig->verbose) {
					PRINT("Found service: '%s'\n", str);
				}
			}
		} //array processing end
		else { /* this is not a header or array but something else */
		}
	} // finished unpacking.

	msgpack_unpacked_destroy(&unpacked_result);

	if (return_status == MSGPACK_UNPACK_PARSE_ERROR) {
		PRINT("The data in the buf is invalid format.\n");
	}
	return unpacked;
}
