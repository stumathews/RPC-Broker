#include "broker_support.h"
#include "common.h"

extern struct ClientRequestRegistration client_request_repository;
extern bool verbose;

struct ClientRequestRegistration *register_client_request(char* op, Location* src, int message_id)
{

    if(verbose) { PRINT("Registering client service request, '%s:%s'(%s)\n", src->address, src->port,  op); }

    ClientReg* client_request_registration = malloc(sizeof(struct ClientRequestRegistration));
    client_request_registration->address = src->address;
    client_request_registration->port = src->port;
    client_request_registration->operation = op;
    client_request_registration->message_id = message_id;

    list_add(&(client_request_registration->list),&(client_request_repository.list));

    return client_request_registration;
}
