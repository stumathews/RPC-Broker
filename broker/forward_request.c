#include "broker_support.h"
#include "common.h"

extern bool verbose_flag;
extern struct ServiceRegistration service_repository;
extern struct ServiceRequestRegistration client_request_repository;

// send the client's service requets to the server that is known to be able to process it
void forward_request(char* buffer, int len, struct sockaddr_in* peerp)
{
    Destination *src = get_sender_address( buffer, len, peerp); 
    Destination *dest = find_server(buffer, len);
    
    char* requested_operation = Alloc(sizeof(char));
    int*  message_id = Alloc( sizeof(int));
    
    *message_id = get_header_int_value( buffer, len, "message-id");
    requested_operation = get_header_str_value(buffer, len, "op"); 

    ClientReg *crreg = register_client_request(requested_operation , src, *message_id );

    if( dest->address == NULL ||  dest->port == NULL ) 
    {
        if(verbose_flag)
            PRINT("No server can process that request\n");
        return;
    }
    
    if(verbose_flag) 
        PRINT("About to send request to service at %s:%s\n", dest->address, dest->port);

    send_request( buffer, len, dest->address, dest->port,verbose_flag);

}
