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

extern char broker_address[30];
extern char broker_port[20];
extern bool verbose;

int add( int one, int two )
{
    
    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "%d%d",one,two);

    send_request(sbuf.data,sbuf.size, broker_address, broker_port, verbose);

    msgpack_sbuffer_destroy(&sbuf);
    
    _return();
}

void echo(char* echo)
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "%s",echo);

    send_request(sbuf.data,sbuf.size, broker_address, broker_port, verbose);

    msgpack_sbuffer_destroy(&sbuf);
    
    _return();
}

char* getBrokerName()
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "");
    
    send_request(sbuf.data,sbuf.size, broker_address, broker_port, verbose);

    msgpack_sbuffer_destroy(&sbuf);

    _return();
}

char* getServerDate()
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, "");
    
    send_request(sbuf.data,sbuf.size, broker_address, broker_port,verbose);

    msgpack_sbuffer_destroy(&sbuf);

    // --
    // Read response from the Broker
    // ---

    char* dummyResult = "11:11:05am";

    // return result (incomplete)
    _return();
}


void _return()
{

}
