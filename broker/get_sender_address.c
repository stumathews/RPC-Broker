#include "broker_support.h"
#include "common.h"

Location* get_sender_address( Packet* packet, struct sockaddr_in* peerp )
{ 
    List* mem_pool = LIST_GetInstance();
    char* reply_port =  get_header_str_value( packet, REPLY_PORT_HDR ); 

    char* sender_address = get_header_str_value( packet, SENDER_ADDRESS_HDR ); 

    Location* addr = Alloc( sizeof( Location ),mem_pool );
    addr->address = sender_address;
    addr->port = reply_port;
    
    return addr; // dangling pointer
}

