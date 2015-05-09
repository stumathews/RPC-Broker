#include "common.h"
#include <stulibc.h>
#include <msgpack.h>

// Send request to broker.
void send_request(char* buffer, int bufsize,char* address, char* port)
{
    // --------------------------
    // SEND PACKED DATA TO BROKER
    // -------------------------
    
    struct sockaddr_in peer;
	SOCKET s;
    
	s = netTcpClient(address,port);
	client( s, &peer, buffer,bufsize );
}
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
// pack the object to send to the broker.
char* pack_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
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
void unpack_request_data(char const* buf, size_t len)
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
