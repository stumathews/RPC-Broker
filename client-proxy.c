#include "server_interface.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <stulibc.h>


void unpack(char const* buf, size_t len);

// called from the client and realised from server_inerface.h
void getServerDate(char* buffer, int length)
{
    // pack the object to send to the broker.

    /* Broker Object format:
        {"op"=>"getServerDate"}
        {"params" => [ {"buffer"=>buffer}, {"length"=>length} ]}}
    
    */

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // pack a map with 1 items
    msgpack_pack_map(&pk,1);

    msgpack_pack_str(&pk, 2);
    msgpack_pack_str_body(&pk, "op", 2);
    msgpack_pack_str(&pk, 13);
    msgpack_pack_str_body(&pk, "getServerDate", 13);
    // pack a map with 1 item
    msgpack_pack_map(&pk,1);
    
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "params", 6);
    // pack an array of 2 items(params)
    msgpack_pack_array(&pk, 2);
    
    msgpack_pack_map(&pk,1);

    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "buffer", 6);
    msgpack_pack_str(&pk, length);
    msgpack_pack_str_body(&pk, buffer, length);
    
    msgpack_pack_map(&pk,1);
    
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "length", 6);
    msgpack_pack_int(&pk, length);

    // send object(sbuf.data) size: sbuf.size to broker and wait for reply
    char* dummyResult = "11:11:05am";
    memcpy(buffer,dummyResult,strlen(dummyResult));
    PRINT("Packed message.\n");
    unpack((const char*)sbuf.data,sbuf.size);
}

void unpack(char const* buf, size_t len)
{
    
    /* buf is allocated by client. */
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&result);

    // Go ahead unpack an object
    ret = msgpack_unpack_next(&result, buf, len, &off);

    // Go and get the rest of all was good
    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        /* Use obj. */
        
        // We have the object
        msgpack_object obj = result.data;
        // print it:

        /* Use obj. */
        msgpack_object_print(stdout, obj);
        printf("\n");

        /* If you want to allocate something on the zone, you can use zone. */
        /* msgpack_zone* zone = result.zone; */
        /* The lifetime of the obj and the zone,  */

        ret = msgpack_unpack_next(&result, buf, len, &off);
    }
    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
        printf("The data in the buf is invalid format.\n");
    }
}


