#include "broker_support.h"
#include "common.h"

extern struct ClientRequestRegistration client_request_repository;
extern bool verbose_flag;

struct ClientRequestRegistration *register_client_request( char* op, Destination* src, int message_id )
{
    if( verbose_flag) {PRINT("Registering client request from '%s:%s' for operation '%s'\n", src->address,src->port,  op);}
    ClientReg* client_request_registration = Alloc( sizeof( struct ClientRequestRegistration) );
    client_request_registration->address = src->address;
    client_request_registration->port = src->port;
    client_request_registration->operation = op;
    client_request_registration->message_id = message_id;

    list_add( &(client_request_registration->list),&(client_request_repository.list)); 
    return client_request_registration;
}
