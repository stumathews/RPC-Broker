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

// Send request to listening broker socket
void client(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length)
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
    
    // wait for a response from the broker?
    //unpack_data((const char*)buffer,length);
    

}


// Send request to broker.
static void send_request(char* buffer, int bufsize)
{
    // --------------------------
    // SEND PACKED DATA TO BROKER
    // -------------------------
    
    struct sockaddr_in peer;
	SOCKET s;
    
	s = netTcpClient(broker_address,broker_port);
	client( s, &peer, buffer,bufsize );
}

// pack the object to send to the broker.
char* pack_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{

    /* ********************
     * Broker Object format:
     * ********************
        
       {"op"=>"getServerDate"}
       {"params" => [ buffer,length ]}

     */
    
    // ---------
    // PACK DATA
    // ----------
    
    
    msgpack_sbuffer_init(sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);
    
    // {"op"=>"getServerDate"}
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 2);
    msgpack_pack_str_body(&pk, "op", 2);
    msgpack_pack_str(&pk, strlen(op));
    msgpack_pack_str_body(&pk, op, strlen(op));
    
    // {"params" => [ {"buffer"=>buffer}, {"length"=>length} ]}}
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "params", 6);
    
    va_list ap;
    va_start(ap,(const char*)fmt);
    char *p, *sval;
    int ival;
    int numargs = 0;

    // we need to know before hand how many arguments we have
    // as we have to make space for that many items in the array
    for( p = fmt;*p;p++)
    {
        if(*p != '%') {
            putchar(*p);
            continue;
        }
        numargs++;
    }
    
    // pack an array of numargs capacity (the params)
    // [ buffer,length ]
    msgpack_pack_array(&pk, numargs);

    // Extract params form param fmt and interpet and pack
    for( p = fmt;*p;p++)
    {
        if(*p != '%') {
            putchar(*p);
            continue;
        }
        switch(*++p)
        {
            case 'd':
                ival = va_arg(ap,int);
                msgpack_pack_int(&pk, ival);
                break;
            case 's':
                sval =  va_arg(ap, char *);
                msgpack_pack_str(&pk,strlen(sval));
                msgpack_pack_str_body(&pk, sval, strlen(sval));
                break;
        }
    }
}

void echo(char* echo)
{

    msgpack_sbuffer sbuf;

    pack_data( &sbuf, (char*)__func__, "%s",echo);
    
    send_request(sbuf.data,sbuf.size);

    msgpack_sbuffer_destroy(&sbuf);
}

char* getBrokerName()
{

    msgpack_sbuffer sbuf;

    pack_data( &sbuf, (char*)__func__, "");
    
    send_request(sbuf.data,sbuf.size);

    msgpack_sbuffer_destroy(&sbuf);
}

// Called from the client and realised from server_inerface.h
void getServerDate(char* buffer,int length)
{

    msgpack_sbuffer sbuf;

    pack_data( &sbuf, (char*)__func__, "%s%d",buffer,length);
    
    send_request(sbuf.data,sbuf.size);

    msgpack_sbuffer_destroy(&sbuf);

    // --
    // Read response from the Broker
    // ---

    char* dummyResult = "11:11:05am";
    memcpy(buffer,dummyResult,strlen(dummyResult));

    // return result (incomplete)
    _return();
}


void _return()
{

}

