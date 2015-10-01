#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

extern bool verbose;
extern char port[MAX_PORT_CHARS];
extern char broker_address[MAX_ADDRESS_CHARS];

// =====================
// SERVICE Registration
// =====================
// Craft a service registration message and send it of fto the broker.
bool service_register_with_broker( char *broker_address, char* broker_port )
{
    ServiceReg *sr = Alloc( sizeof( ServiceReg ) );
    sr->address = "localhost"; // TODO: Get our actual IP address
    sr->port = port;
    
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    pack_map_int("request_type",REQUEST_REGISTRATION,&pk);
    pack_map_str("sender-address",sr->address,&pk);
    pack_map_str("reply-port",sr->port,&pk);
    pack_map_str("service-name","theServiceName",&pk);

    // pull in the services defined in the server code: server.c
    extern char* services[];
    char* service = services[0];
    int i = 0;
    while( services[i] != NULL )
    {
        if(verbose)
            PRINT("Service %s.\n", services[i]);
        i++;
    }
    pack_map_int("services-count",i,&pk);
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 8);
    msgpack_pack_str_body(&pk, "services", 8);
    msgpack_pack_array(&pk, i);

    if( verbose )
        PRINT("num services %d\n",i);

    // pack the services into the protocol message
    while( i >= 0 )
    {
        if( !STR_IsNullOrEmpty(services[i] ))
        {
            if(verbose)
                PRINT("service packed is %s\n", services[i]);
            msgpack_pack_str(&pk, strlen(services[i]));
            msgpack_pack_str_body(&pk, services[i], strlen(services[i]));
        }
        i--;    
    }
    // send registration message to broker
    Packet pkt; pkt.buffer = sbuf.data; pkt.len = sbuf.size;
    send_request( &pkt, broker_address, broker_port,verbose);
    msgpack_sbuffer_destroy(&sbuf);
}
