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


extern char broker_address[30];
extern char broker_port[20];

// Send request to listening broker socket
void send_request(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length)
{
    struct packet pkt;
    pkt.len = htonl(length);
    pkt.buffer = buffer;

    // Send size of packet
    int rc = 0; 
    if( (rc = send(s, (char*) &pkt.len , sizeof(u_int32_t),0)) < 0 )
        netError(1, errno, "failed to send size\n");
    pkt.len = ntohl(pkt.len);
    printf("sent %d bytes\n",rc);
    printf("send length as %u\n",pkt.len);

    // send packet
    if( (rc = send(s, buffer, pkt.len,0)) < 0 )
        netError(1,errno,"failed to send packed data\n");
    printf("Sent %d bytes:\n",rc);
    printf("buffer length %d\n",length);

    //debugging:
    //write(1,buffer,length);
    //unpack_data((const char*)buffer,length);
}


// call send_request
static void client( SOCKET s, struct sockaddr_in* peerp, char* buffer, int length)
{
    // Send request to broker.
    send_request(s,peerp,buffer,length);
}


// Called from the client and realised from server_inerface.h
void getServerDate(char* buffer,int length)
{

    // ---------
    // PACK DATA
    // ----------
    
    // pack the object to send to the broker.

    /* 
     * Broker Object format:
        
       {"op"=>"getServerDate"}
        {"params" => [ {"buffer"=>buffer}, {"length"=>length} ]}}
    
    */
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // {"op"=>"getServerDate"}
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 2);
    msgpack_pack_str_body(&pk, "op", 2);
    msgpack_pack_str(&pk, 13);
    msgpack_pack_str_body(&pk, "getServerDate", 13);
    
    // {"params" => [ {"buffer"=>buffer}, {"length"=>length} ]}}
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "params", 6);
   

    // pack an array of 2 (the params)
    
    // [ {"buffer"=>buffer}, {"length"=>length} ]}}
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

    // --------------------------
    // SEND PACKED DATA TO BROKER
    // -------------------------

    struct sockaddr_in peer;
	SOCKET s;
    
	s = netTcpClient(broker_address,broker_port);
    
    // call blocking network function, send.
	client( s, &peer, sbuf.data,sbuf.size );


    // --
    // Read response from the Broker
    // ---

    // --??
    // SIMULATE RESULT
    // --
    char* dummyResult = "11:11:05am";
    memcpy(buffer,dummyResult,strlen(dummyResult));

    // cleanup
    msgpack_sbuffer_destroy(&sbuf);

    // return result (incomplete)
    _return();
}


void _return()
{

}

