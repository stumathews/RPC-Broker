#include "broker_support.h"
#include "common.h"

Destination* get_sender_address( Packet packet, struct sockaddr_in* peerp )
{
    char* reply_port = Alloc(sizeof(char) * MAX_PORT_CHARS);
    reply_port =  get_header_str_value( packet, REPLY_PORT_HDR ); 

    char* sender_address = Alloc( sizeof(char) * MAX_ADDRESS_CHARS);
    sender_address = get_header_str_value( packet, SENDER_ADDRESS_HDR ); 

    Destination* addr = Alloc( sizeof( Destination ) );
    addr->address = sender_address;
    addr->port = reply_port;
    
    return addr;
}

