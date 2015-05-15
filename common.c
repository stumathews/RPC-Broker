#include "common.h"
#include <stulibc.h>
#include <msgpack.h>

void pack_map_str( char* key, char* value, msgpack_packer* pk);
void pack_map_int(char* key, int ival,msgpack_packer* pk );

bool service_register_with_broker( char *broker_address, char* broker_port )
{
    ServiceReg *sr = Alloc( sizeof( ServiceReg ) );
    sr->address = "localhost";
    sr->port = "9090";
    
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    pack_map_int("request_type",REQUEST_REGISTRATION,&pk);
    pack_map_str("sender-address","localhost",&pk);
    pack_map_str("reply-port","8080",&pk);
    pack_map_str("service-name","theServiceName",&pk);

    char* services[] = {"service1","service2","server3","service4",NULL };
    char* service = services[0];
    int i = 0;
    while( services[i] != NULL )
    {
        PRINT("Service %s.\n", services[i]);
        i++;
    }
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 8);
    msgpack_pack_str_body(&pk, "services", 8);
    msgpack_pack_array(&pk, i);

    PRINT("num services %d\n",i);
    while( i >= 0 )
    {
        if( !STR_IsNullOrEmpty(services[i] ))
        {
            PRINT("service packed is %s\n", services[i]);
            msgpack_pack_str(&pk, strlen(services[i]));
            msgpack_pack_str_body(&pk, services[i], strlen(services[i]));
        }
        i--;    
    }
    unpack_request_data(sbuf.data, sbuf.size);
    send_request( sbuf.data, sbuf.size, broker_address, broker_port);
    msgpack_sbuffer_destroy(&sbuf);

}

// Send request to broker.
int send_request(char* buffer, int bufsize,char* address, char* port)
{
    // --------------------------
    // SEND PACKED DATA TO BROKER
    // -------------------------
    
    struct sockaddr_in peer;
	SOCKET s;
    
	s = netTcpClient(address,port);
	return client( s, &peer, buffer,bufsize );
}
// Send request to listening broker socket
int client(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length)
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
    return rc;

}


void pack_map_int(char* key, int ival,msgpack_packer* pk )
{
    msgpack_pack_map(pk,1);
    msgpack_pack_str(pk, strlen(key));
    msgpack_pack_str_body(pk, key, strlen(key));
    msgpack_pack_int(pk, ival);

}
// Packs a key/value pair like this: {"key"=>"valuet"}
// where key is a string and value if an char
void pack_map_str( char* key, char* value, msgpack_packer* pk)
{

    msgpack_pack_map(pk,1);
    msgpack_pack_str(pk, strlen(key));
    msgpack_pack_str_body(pk, key, strlen(key));
    msgpack_pack_str(pk, strlen(value));
    msgpack_pack_str_body(pk, value, strlen(value));
}

// pack the object to send to the broker.
char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{

    
    // ---------
    // PACK DATA
    // ----------
    
    msgpack_sbuffer_init(sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);
    
    pack_map_int("request_type",0,&pk);
    pack_map_str("op",op,&pk);

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

        // Read all the headers
        // read the specific structures for this request type.

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

enum RequestType determine_request_type(struct packet* pkt)
{
        
    /* buf is allocated by client. */
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    msgpack_unpacked_init(&result);

    // Go ahead unpack an object
    ret = msgpack_unpack_next(&result, pkt->buffer, pkt->len, &off);
    if (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;

        msgpack_object_print(stdout, obj);

        if( obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER || obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER )
        {            
            return obj.via.u64;
        }
        else
        {
            PRINT("Expected reqyest type in protocol.\n");
            exit(1);
        }
    }

    msgpack_unpacked_destroy(&result);
}
