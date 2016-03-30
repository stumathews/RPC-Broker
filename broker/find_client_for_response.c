#include "broker_support.h"
#include "common.h"

extern struct ClientRequestRegistration client_request_repository;

Location* find_client_for_response(Packet *packet, Location* dest, struct BrokerConfig *brokerConfig)
{
    struct list_head *pos, *q;
    char* requested_operation;
    int*  message_id;
    ClientReg *crreg_entry;

    if(list_empty( &client_request_repository.list)) {
        PRINT("No client requests registered in broker\n");
    }

    requested_operation = malloc(sizeof(char));
    message_id = malloc( sizeof(int));

    list_for_each(pos, &client_request_repository.list) {
        crreg_entry  = list_entry( pos, struct ClientRequestRegistration, list);
        *message_id = get_header_int_value( packet, MESSAGE_ID_HDR);
        requested_operation = get_header_str_value(packet, OPERATION_HDR);
        if(*message_id == crreg_entry->message_id && STR_Equals(requested_operation, crreg_entry->operation)) {
            dest->address = crreg_entry->address;
            dest->port = crreg_entry->port;
            if(brokerConfig->verbose) {
            	DBG("found client at %s:%s\n", dest->address, dest->port);
            }
            list_del(&crreg_entry->list);
            return dest;
        }
    }
    free(requested_operation);
    free(message_id);
    return dest;
}
