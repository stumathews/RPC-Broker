#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

void call_marshelResponse_send(int message_id, char* operation,
		char* broker_address, char* broker_port, bool verbose, char** params);

/**
 * @brief Unpackets service request, extracts parameters and calls server functions and sends back the response to the broker
 *
 * @param buffer the service request from the client/broker
 * @param buflen the length of the data
 * @return void
 */
void unpack_marshal_call_send(char* buffer, int buflen, Details brokerDetails,
		Config brokerConfig) {
	msgpack_unpacked unpacked_result;
	msgpack_unpack_return return_status;
	size_t off = 0;
	int i = 0;
	void** params = 0;
	int message_id;

	if (brokerConfig.verbose)
		PRINT("broker address is %s, broker port is %s verbose is %d\n",
				brokerDetails.address, brokerDetails.port,
				brokerConfig.verbose);

	msgpack_unpacked_init(&unpacked_result);

	while ((return_status = msgpack_unpack_next(&unpacked_result, buffer,
			buflen, &off)) == MSGPACK_UNPACK_SUCCESS) {
		char header_name[MAX_HEADER_NAME_SIZE];
		char* ptrOperation;

		msgpack_object obj = unpacked_result.data;
		memset(header_name, '\0', MAX_HEADER_NAME_SIZE);
		msgpack_object val = extract_header(&obj, header_name);

		if (STR_Equals(OPERATION_HDR, header_name)
				&& val.type == MSGPACK_OBJECT_STR) {
			char* ptrStr = (char*) malloc(
					sizeof(char) * (size_t) val.via.str.size + 1);
			copyString(val.via.str.size, &val.via.str, ptrStr);
			ptrOperation = ptrStr;
		} else if (STR_Equals(MESSAGE_ID_HDR, header_name)
				&& val.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
			int64_t ival = val.via.i64;
			message_id = ival;
		} else if (val.type == MSGPACK_OBJECT_ARRAY) {
			params = calloc(val.via.array.size, sizeof(void*));
			for (int i = 0; i < val.via.array.size; i++) {
				msgpack_object_type param_type = val.via.array.ptr[i].type;
				msgpack_object param = val.via.array.ptr[i];
				if (param_type == MSGPACK_OBJECT_STR) {
					int alloc_size = param.via.str.size + 1;
					char* ptrStr = malloc(sizeof(char) * alloc_size);
					copyString(param.via.str.size, &param.via.str, ptrStr);
					params[i] = ptrStr;
				} else if (param_type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
					int64_t *pival = malloc(sizeof(int));
					*pival = param.via.i64;
					params[i] = pival;
				}
			}
			// Now arrange for the service call to be invoked and marshal the parmeters into the function call
			call_marshelResponse_send(message_id, ptrOperation,
					brokerDetails.address, brokerDetails.port,
					brokerConfig.verbose, (char**) params);
		}
	} // finished unpacking.
	if (return_status == MSGPACK_UNPACK_PARSE_ERROR) {
		PRINT("The data in the buf is invalid format.\n");
	}
	msgpack_unpacked_destroy(&unpacked_result);
}
