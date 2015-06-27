#include "broker_support.h"
#include "common.h"

Destination* get_sender_address( char* buffer, int len, struct sockaddr_in* peerp )
{
    char* reply_port = Alloc(sizeof(char) * MAX_PORT_CHARS);
    reply_port =  get_header_str_value( buffer, len, "reply-port" ); 

    char* sender_address = Alloc( sizeof(char) * MAX_ADDRESS_CHARS);
    sender_address = get_header_str_value( buffer, len, "sender-address" ); 

    Destination* addr = Alloc( sizeof( Destination ) );
    addr->address = sender_address;
    addr->port = reply_port;
    
    struct in_addr address = peerp->sin_addr; 
    unsigned long srcAddr = address.s_addr; // load with inet_pton();
    unsigned short sin_port = peerp->sin_port; //load with htons
    return addr;
}
