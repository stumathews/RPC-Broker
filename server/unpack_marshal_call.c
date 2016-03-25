#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

extern bool verbose;
extern char broker_address[MAX_ADDRESS_CHARS];
extern char broker_port[MAX_PORT_CHARS];
void call_marshelResponse_send(int message_id, char* operation, char* broker_address, char* broker_port, bool verbose, char** params);

void copyString(int str_len, const msgpack_object_str* from, char* to) {
	memset(to, '\0', str_len);
	to[str_len] = '\0';
	strncpy(to, from->ptr, str_len);
}

/**
 * @brief Unpackets service request, extracts parameters and calls server functions and sends back the response to the broker
 * 
 * @param buffer the service request from the client/broker
 * @param buflen the length of the data
 * @return void
 */
void unpack_marshal_call_send(char* buffer, int buflen)
{
    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&unpacked_result);
    void** params = 0;
    int message_id;
    return_status = msgpack_unpack_next(&unpacked_result, buffer, buflen, &off);
    
    while (return_status == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = unpacked_result.data;
        // stores the protocol header
        char header_name[MAX_HEADER_NAME_SIZE];
        // stores the service request operation name
        char* operation;
        memset(header_name, '\0', MAX_HEADER_NAME_SIZE);
        msgpack_object val = extract_header(&obj, header_name);

        // Extract the operation name to call.
        if(STR_Equals( "op", header_name) && val.type == MSGPACK_OBJECT_STR) {
            char* str = malloc(sizeof(char) * val.via.str.size);
            copyString(val.via.str.size, &val.via.str, str);
            operation = str;
        } else if(STR_Equals(MESSAGE_ID_HDR, header_name) && val.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
        	//param is an int
            int64_t ival = val.via.i64;
            message_id = ival;
        } else if(val.type == MSGPACK_OBJECT_ARRAY) {
        	// Get list of parameters for the operation
            params = malloc( sizeof(void*) * val.via.array.size);
            for(int i = 0; i < val.via.array.size;i++) {
                msgpack_object_type param_type = val.via.array.ptr[i].type;
                msgpack_object param = val.via.array.ptr[i];
                params[i] = NULL;
                if(param_type == MSGPACK_OBJECT_STR) {
                	//param is a char*
                    char* str = malloc(sizeof(char) * param.via.str.size + 1);
                    memset(str, '\0', param.via.str.size + 1);
                    strncpy(str, param.via.str.ptr, param.via.str.size);
                    params[i] = str;
                } else if(param_type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
                	//param is an int
                     int64_t *pival = malloc(sizeof(int));
                     *pival = param.via.i64;
                     params[i] = pival;
                }
            }
            // Now arrange for the service call to be invoked and marshal the parmeters into the function call
            call_marshelResponse_send(message_id, operation, broker_address, broker_port, verbose,(char**)params);
        }
        return_status = msgpack_unpack_next(&unpacked_result, buffer, buflen, &off);
    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        PRINT("The data in the buf is invalid format.\n");
    }
}
