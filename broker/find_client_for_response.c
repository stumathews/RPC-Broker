#include "broker_support.h"
#include "common.h"

extern bool verbose_flag;
extern struct ClientRequestRegistration client_request_repository;

Location* find_client_for_response(Packet *packet)
{
    char* op_name = get_op_name(packet);
    Location* dest = Alloc(sizeof(Location));

    struct list_head *pos, *q;

    if( list_empty( &client_request_repository.list ))
    {
        PRINT("No client requests registered in broker\n");
    }

    list_for_each(pos, &client_request_repository.list)
    {
        ClientReg *crreg_entry  = list_entry( pos, struct ClientRequestRegistration, list );
        char* requested_operation = Alloc(sizeof(char));
        int*  message_id = Alloc( sizeof(int));
    
        *message_id = get_header_int_value( packet, MESSAGE_ID_HDR);
        requested_operation = get_header_str_value(packet, "op"); 
        if( *message_id == crreg_entry->message_id && STR_Equals(requested_operation, crreg_entry->operation ))
        {
            dest->address = crreg_entry->address;
            dest->port = crreg_entry->port;
            if( verbose_flag ) { PRINT("found client at %s:%s\n", dest->address, dest->port);}
            list_del( &crreg_entry->list);
            return dest;
        }       

       
    }

    return dest; //dangling pointer
}
