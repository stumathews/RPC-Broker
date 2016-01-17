#include "broker_support.h"
#include "common.h"

extern bool verbose;
extern struct ClientRequestRegistration client_request_repository;

Location* find_client_for_response(Packet *packet, Location* dest)
{
    char* op_name = get_op_name(packet);
    struct list_head *pos, *q;
    List* mem_pool = LIST_GetInstance();

    if( list_empty( &client_request_repository.list ))
    {
        PRINT("No client requests registered in broker\n");
    }

    list_for_each(pos, &client_request_repository.list)
    {
        ClientReg *crreg_entry  = list_entry( pos, struct ClientRequestRegistration, list );
        char* requested_operation = Alloc(sizeof(char),mem_pool);
        int*  message_id = Alloc( sizeof(int),mem_pool);
    
        *message_id = get_header_int_value( packet, MESSAGE_ID_HDR);
        requested_operation = get_header_str_value(packet, "op"); 
        if( *message_id == crreg_entry->message_id && STR_Equals(requested_operation, crreg_entry->operation ))
        {
            dest->address = crreg_entry->address;
            dest->port = crreg_entry->port;
            if(verbose) {
            	DBG("found client at %s:%s\n", dest->address, dest->port);
            }
            //list_del( &crreg_entry->list);
            return dest;
        }       

    }

    return dest;
}
