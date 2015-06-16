#include "broker_support.h"
#include "common.h"

extern char port[20];
extern bool verbose;
extern bool waitIndef;
extern struct ServiceRegistration service_repository;

// send the client's service requets to the server that is known to be able to process it
void forward_request(char* buffer, int len)
{
    Destination *dest = Alloc(sizeof(Destination));
    find_server(buffer, len, dest );

    if( dest->address == NULL ||  dest->port == NULL ) 
    {
        if(verbose)
            PRINT("No server can process that request\n");
        return;
    }
    
    if(verbose) 
        PRINT("About to send request to service at %s:%s\n", dest->address, dest->port);

    send_request( buffer, len, dest->address, dest->port,verbose);
}
