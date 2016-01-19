#include "broker_support.h"
#include "common.h"

extern bool verbose;
extern struct ServiceRegistration service_repository;
extern struct ServiceRequestRegistration client_request_repository;

void forward_request_to_server(Packet* packet, Location* src)
{
    List* mem_pool = LIST_GetInstance();
    Location *dest = find_server_for_request(packet);
    
    char* requested_operation = Alloc(sizeof(char), mem_pool);
    int*  message_id = Alloc(sizeof(int),mem_pool);
    

    *message_id = get_header_int_value( packet, MESSAGE_ID_HDR);
    requested_operation = get_header_str_value(packet, OPERATION_HDR); 

    ClientReg *crreg = register_client_request(requested_operation , src, *message_id );

    if( dest->address == NULL ||  dest->port == NULL ) 
    {
        PRINT("No registered server available to process request for operation '%s' from host '%s'. \n", requested_operation, src->address);
        return;
    }
    
    if(verbose) 
        PRINT("About to forward request to %s:%s\n", dest->address, dest->port);

    send_request( packet, dest->address, dest->port, verbose);
}
