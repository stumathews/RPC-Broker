#include "broker_support.h"
#include "common.h"

extern bool verbose_flag;
extern struct ServiceRegistration service_repository;

// send the client's service requets to the server that is known to be able to process it
void forward_request(char* buffer, int len, struct sockaddr_in* peerp)
{
    Destination *dest = find_server(buffer, len);
    struct in_addr address = peerp->sin_addr; 
    unsigned long srcAddr = address.s_addr; // load with inet_pton();
    unsigned short sin_port = peerp->sin_port; //load with htons
    PRINT("Request from: %s:%d", inet_ntoa(address),  ntohs(sin_port) );

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
