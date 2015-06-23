#include "common.h"
#include <stulibc.h>
#include <msgpack.h>

#define REQUEST_TYPE_IDENT "request_type"



// Send request to broker.
int send_request(char* buffer, int bufsize,char* address, char* port, bool verbose)
{
    // --------------------------
    // SEND PACKED DATA TO BROKER
    // -------------------------
   
    if(verbose)
    {
        PRINT("Size to send: %d\n",bufsize);
    }

    struct sockaddr_in peer;
	SOCKET s;
    
	s = netTcpClient(address,port);
	return client( s, &peer, buffer,bufsize,verbose );
}
// Send request to listening broker socket
int client(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length, bool verbose)
{
    struct packet pkt;
    pkt.len = htonl(length);
    pkt.buffer = buffer;

    // Send size of packet
    int rc = 0; 
    if( (rc = send(s, (char*) &pkt.len , sizeof(u_int32_t),0)) < 0 )
        netError(1, errno, "failed to send size\n");
    pkt.len = ntohl(pkt.len);
    if( verbose )
    {
        PRINT("Sent %d bytes(size of packet)\n",rc);
        PRINT("send pkt length as %u\n",pkt.len);
    }

    // send packet
    if( (rc = send(s, buffer, pkt.len,0)) < 0 )
        netError(1,errno,"failed to send packed data\n");
    if( verbose )
    {
        PRINT("Sent packet %d bytes(packet buffer)\n",rc);
        PRINT("buffer packet  length %d\n",length);
    }
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

char* pack_client_response_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{
    // ---------
    // PACK DATA
    // ----------
    
    msgpack_sbuffer_init(sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);
    
    pack_map_int("request_type",REQUEST_SERVICE_RESPONSE,&pk);

    pack_map_str("op",op,&pk);

    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "reply", 5);
    
    va_list ap;
    va_start(ap,(const char*)fmt);
    char *p, *sval;
    int ival;
    int numargs = 0;

    for( p = fmt;*p;p++)
    {
        if(*p != '%') {
            putchar(*p);
            continue;
        }
        numargs++;
    }
    
    msgpack_pack_array(&pk, numargs);

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
                int sval_len = strlen(sval);
                msgpack_pack_str(&pk,sval_len);
                msgpack_pack_str_body(&pk, sval, sval_len);
                break;
        }
    }
}
// pack the object to send to the broker. (client's service request)
char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{


    // ---------
    // PACK DATA
    // ----------

    msgpack_sbuffer_init(sbuf);

    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

    pack_map_int("request_type",REQUEST_SERVICE,&pk);

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
                int sval_len = strlen(sval);
                msgpack_pack_str(&pk,sval_len);
                msgpack_pack_str_body(&pk, sval, sval_len);
                break;
        }
    }
}
void unpack_data(char const* buf, size_t len, bool verbose)
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
        msgpack_object obj = result.data;
        msgpack_object_print(stdout, obj);
        printf("\n");
        ret = msgpack_unpack_next(&result, buf, len, &off);
    }
    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
        printf("The data in the buf is invalid format.\n");
    }
}

msgpack_object extract_header( msgpack_object* obj, char* header_buffer )
{
    // msgpack_object.type (msgpack_object_type)
    // msgpack_object.via (msgpack_object_union)
    if( obj->type == MSGPACK_OBJECT_MAP )
    {            
        int count = obj->via.map.size; // How many items in this map? 
        if( count != 1 )
        {
            PRINT("Expected count of items in map to be 1. Not the case: %d\n", count);
            exit(1);
        }

        struct msgpack_object_kv* pairs = obj->via.map.ptr; // all the key/value pairs in this map

        msgpack_object key = pairs[0].key;
        msgpack_object val = pairs[0].val;

        char key_name[key.via.str.size];
        strncpy(key_name, key.via.str.ptr, key.via.str.size);
        key_name[key.via.str.size] = '\0';
        strncpy(header_buffer, key_name, key.via.str.size);

        return val;
    }
    else
    {
        PRINT("Expected request type to be a map.\n");
        exit(1);
    }
}

void unpack_headers( struct packet* pkt )
{

}

enum RequestType determine_request_type(struct packet* pkt)
{
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    msgpack_unpacked_init(&result);

    // unpack just one object
    ret = msgpack_unpack_next(&result, pkt->buffer, pkt->len, &off);
    if (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;

        char header_name[20]; // assume that the protocol states that the header_name can not be longer than 20 characters
        memset( header_name, '\0', 20);

        msgpack_object val = extract_header( &obj, header_name );

        if(val.type == MSGPACK_OBJECT_POSITIVE_INTEGER && strcmp(REQUEST_TYPE_IDENT,header_name) == 0 )
        {
            return val.via.i64;
        }
        else
        {
            PRINT("Expecting request_type. got header name as '%s' and value as '%d'\n.",header_name, val.via.i64);
            exit(1);
        }
        
    }
    else { PRINT("pack failed\n"); }

    msgpack_unpacked_destroy(&result);
}
