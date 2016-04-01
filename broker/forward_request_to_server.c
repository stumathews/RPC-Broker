#include "broker_support.h"
#include "common.h"


void forward_request_to_server(Packet* packet, Location* src, struct BrokerConfig *brokerConfig)
{
	ClientReg *crreg;
    Location *dest;
    char* requested_operation;
    int*  message_id;
    
    dest = find_server_for_request(packet);
    requested_operation = malloc(sizeof(char));
    message_id = malloc(sizeof(int));
    *message_id = get_header_int_value( packet, MESSAGE_ID_HDR);
    requested_operation = get_header_str_value(packet, OPERATION_HDR); 
	crreg = register_client_request(requested_operation , src, *message_id, brokerConfig );

    if(dest->address == NULL ||  dest->port == NULL) {
        PRINT("No registered server available to process request for operation '%s' from host '%s'. \n", requested_operation, src->address);
        return;
    }
    
    if(brokerConfig->verbose) {
        PRINT("About to forward request to %s:%s\n", dest->address, dest->port);
    }

    send_request(packet, dest->address, dest->port, brokerConfig->verbose);
}
