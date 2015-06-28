/**
 * Client Proxy for clients
 */

#include "server_interface.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <stulibc.h>
#include <stdarg.h>

extern char broker_address[MAX_ADDRESS_CHARS];
extern char broker_port[MAX_PORT_CHARS];
extern bool verbose;
extern char wait_response_port[MAX_PORT_CHARS];

int add( int one, int two )
{
    
    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "%d%d",one,two);

    Packet pkt; pkt.buffer = sbuf.data; pkt.len = sbuf.size; 

    Packet *result = send_and_receive( pkt, broker_address, broker_port, verbose, wait_response_port );

    msgpack_sbuffer_destroy(&sbuf);
    
    return  get_header_int_value(*result, REPLY_HDR);
    
}

char* echo(char* echo)
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "%s",echo);

    Packet pkt; pkt.buffer = sbuf.data; pkt.len = sbuf.size; 

    Packet *result = send_and_receive( pkt, broker_address, broker_port, verbose, wait_response_port );

    msgpack_sbuffer_destroy(&sbuf);
    
    return  get_header_str_value(*result, REPLY_HDR);
    
}

char* getBrokerName()
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "");

    Packet pkt; pkt.buffer = sbuf.data; pkt.len = sbuf.size; 

    Packet *result = send_and_receive( pkt, broker_address, broker_port, verbose, wait_response_port );

    msgpack_sbuffer_destroy(&sbuf);

    return  get_header_str_value(*result, REPLY_HDR);

}

char* getServerDate()
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "");
   
    Packet pkt; pkt.buffer = sbuf.data; pkt.len = sbuf.size; 
    
    Packet *result = send_and_receive( pkt, broker_address, broker_port, verbose, wait_response_port );

    msgpack_sbuffer_destroy(&sbuf);

    return  get_header_str_value(*result, REPLY_HDR);
}



