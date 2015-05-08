#include "server_interface.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <stulibc.h>

    // pack the object to send to the broker.

    /* Broker Object format:
        {"op"=>"getServerDate"}
        {"params" => [ {"buffer"=>buffer}, {"length"=>length} ]}}
    
    */

void unpack(char const* buf, size_t len);

static void client( SOCKET s, struct sockaddr_in* peerp, char* buffer, int length)
{
        // Send the size of buffer, then buffer to the broker.
        struct packet pkt;
        pkt.len = htonl(length);
        pkt.buffer = buffer;
    
        int rc = 0; 
        if( (rc = send(s, (char*) &pkt.len , sizeof(u_int32_t),0)) < 0 )
            netError(1, errno, "failed to send size\n");
        pkt.len = ntohl(pkt.len);
        printf("sent %d bytes\n",rc);
        printf("send length as %u\n",pkt.len);
        
        if( (rc = send(s, buffer, pkt.len,0)) < 0 )
            netError(1,errno,"failed to send packed data\n");
        printf("Sent %d bytes:\n",rc);
        printf("buffer length %d\n",length);
        write(1,buffer,length);
        unpack((const char*)buffer,length);
}

// Called from the client and realised from server_inerface.h

void getServerDate(char* buffer, int length)
{

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    //{"op"=>"getServerDate"}
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 2);
    msgpack_pack_str_body(&pk, "op", 2);
    msgpack_pack_str(&pk, 13);
    msgpack_pack_str_body(&pk, "getServerDate", 13);
    
    //{"params" => [ {"buffer"=>buffer}, {"length"=>length} ]}}
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "params", 6);
   

    //[ {"buffer"=>buffer}, {"length"=>length} ]}}
    // pack an array of 2 (params)
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

    struct sockaddr_in peer;
	SOCKET s;

    
    // Establish a connection(calls connect) and return the socket that represents that connection.
	s = netTcpClient("localhost","9090");
    
    // call blocking network functions
	client( s, &peer, sbuf.data,sbuf.size );


    char* dummyResult = "11:11:05am";
    memcpy(buffer,dummyResult,strlen(dummyResult));

    PRINT("Packed message.\n");
    unpack((const char*)sbuf.data,sbuf.size);

    msgpack_sbuffer_destroy(&sbuf);
	EXIT( 0 );
}



