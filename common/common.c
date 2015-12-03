#include "common.h"
#include <stulibc.h>
#include <msgpack.h>



/**
 * @brief Send request
 * 
 * @param packet data to send
 * @param address address to send data to
 * @param port port to connect to on address
 * @param verbose true if this function should report verbose messages
 * @return int number of bytes sent
 */
int send_request(Packet *packet,char* address, char* port, bool verbose)
{
    if(verbose)
    {
        PRINT("Send request; Size to send: %d\n", packet->len);
    }

    struct sockaddr_in peer;
	SOCKET s;
	s = netTcpClient(address,port);
	return client( s, &peer, packet,verbose );
}

/**
 * @brief Send request to listening broker socket
 * 
 * @param s socket
 * @param peerp peerp 
 * @param packet packet to send
 * @param verbose true if verbose messages ar eallowed
 * @return int number of bytes sent
 */
int client(SOCKET s, struct sockaddr_in* peerp, Packet *packet, bool verbose)
{
    struct Packet pkt;
    pkt.len = htonl(packet->len);
    pkt.buffer = packet->buffer;

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
    if( (rc = send(s, pkt.buffer, pkt.len,0)) < 0 )
        netError(1,errno,"failed to send packed data\n");
    if( verbose )
    {
        PRINT("Sent packet %d bytes(packet buffer)\n",rc);
        PRINT("buffer packet  length %d\n",pkt.len);
    }
    return rc;
}


/**
 * @brief Packs a int header
 * 
 * @param key header name
 * @param ival value
 * @param pk msgpack_packer
 * @return void
 */
void pack_map_int(char* key, int ival, msgpack_packer* pk )
{
    msgpack_pack_map(pk,1);
    msgpack_pack_str(pk, strlen(key));
    msgpack_pack_str_body(pk, key, strlen(key));
    msgpack_pack_int(pk, ival);

}

/**
 * @brief Packs a key/value pair like this: {"key"=>"valuet"}
where key is a string and value if an char
 * 
 * @param key header name
 * @param value header value
 * @param pk msgpack_packer address
 * @return void
 */
void pack_map_str( char* key, char* value, msgpack_packer* pk)
{

    msgpack_pack_map(pk,1);
    msgpack_pack_str(pk, strlen(key));
    msgpack_pack_str_body(pk, key, strlen(key));
    msgpack_pack_str(pk, strlen(value));
    msgpack_pack_str_body(pk, value, strlen(value));
}

/**
 * @brief Displays the contents of the protocol messages
 * 
 * @param packet the protocol messages
 * @param verbose true if should verbose log messages in this function
 * @return void
 */
void unpack_data(Packet* packet, bool verbose)
{
    if( packet->len == 0 ) { return; } 
    /* buf is allocated by client. */
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&result);

    // Go ahead unpack an object
    ret = msgpack_unpack_next(&result, packet->buffer, packet->len, &off);

    // Go and get the rest of all was good
    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;
        msgpack_object_print(stdout, obj);
        
        if( obj.type != MSGPACK_OBJECT_NIL) { printf("\n"); }
        
        ret = msgpack_unpack_next(&result, packet->buffer, packet->len, &off);
    }
    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
        PRINT("The data in the buf is invalid format.\n");
    }
}

/**
 * @brief Gets the msgpack object for the named header
 * 
 * @param obj the source to extract from
 * @param header_buffer the header name to look
 * @return msgpack_object the result
 */
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


/**
 * @brief Determines the request-type header type
 * 
 * @param pkt the protocl message
 * @return RequestType the determined request type
 */
enum RequestType determine_request_type(struct Packet* pkt)
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

        if(val.type == MSGPACK_OBJECT_POSITIVE_INTEGER && strcmp(REQUEST_TYPE_HDR,header_name) == 0 )
        {
            return val.via.i64;
        }
        else
        {
            PRINT("Expecting '%s' - got header name as '%s' and value as '%d'\n.",REQUEST_TYPE_HDR, header_name, val.via.i64);
            exit(1);
        }
        
    }
    else { PRINT("pack failed\n"); }

    msgpack_unpacked_destroy(&result);
}

/**
 * @brief Gets the integer value of a header
 * 
 * @param packet protocol message
 * @param look_header_name header name to look for
 * @return int result
 */
int get_header_int_value (Packet* packet, char* look_header_name )
{
    size_t off = 0;
    int i = 0;
    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    msgpack_unpacked_init(&unpacked_result);
    int return_value = 0;

    return_status = msgpack_unpack_next(&unpacked_result, packet->buffer, packet->len, &off);

    while (return_status == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = unpacked_result.data;

        char header_name[MAX_HEADER_NAME_SIZE];
        memset(header_name, '\0', MAX_HEADER_NAME_SIZE);

        msgpack_object val = extract_header( &obj, header_name);
        if(val.type == MSGPACK_OBJECT_POSITIVE_INTEGER && strcmp(look_header_name,header_name) == 0 )
        {
                return_value = val.via.i64;
        }
        else
        {
            // this is not a header or array but something else
        }
        return_status = msgpack_unpack_next(&unpacked_result, packet->buffer, packet->len, &off);
    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        PRINT("The data in the buf is invalid format.\n");
    }
    return return_value;
}

/**
 * @brief Gets the string value associated with the named header
 * 
 * @param packet protocol message
 * @param look_header_name header name to look for
 * @return char* the result value
 */
char* get_header_str_value (Packet* packet, char* look_header_name )
{
    size_t off = 0;
    int i = 0;
    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    msgpack_unpacked_init(&unpacked_result);
    char* str;
    return_status = msgpack_unpack_next(&unpacked_result, packet->buffer, packet->len, &off);

    while (return_status == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = unpacked_result.data;

        char header_name[MAX_HEADER_NAME_SIZE];
        memset(header_name, '\0', MAX_HEADER_NAME_SIZE);

        msgpack_object val = extract_header( &obj, header_name);

        if( val.type == MSGPACK_OBJECT_STR &&  STR_Equals( look_header_name, header_name ) == true)
        {
                int str_len = val.via.str.size;
                str = Alloc( str_len);

                memset( str, '\0', str_len);
                str[str_len] = '\0';
                strncpy(str, val.via.str.ptr,str_len); 
                break;
        }
        else
        {
            // this is not a header or array but something else
        }
        return_status = msgpack_unpack_next(&unpacked_result, packet->buffer, packet->len, &off);
    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        PRINT("The data in the buf is invalid format.\n");
    }
    return str; // dangling pointer
}
/**
 * @brief Find the operation name in the protocol message
 * 
 * @param packet the protocol message
 * @return char* the operation found
 */
char* get_op_name( Packet* packet)
{

    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    size_t off = 0;
    char header_name[MAX_HEADER_NAME_SIZE] = {0};
    
    msgpack_unpacked_init(&unpacked_result);

    return_status = msgpack_unpack_next(&unpacked_result, packet->buffer, packet->len, &off);

    while (return_status == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object result_data = unpacked_result.data;
        
        memset(header_name, '\0', MAX_HEADER_NAME_SIZE);
        
        msgpack_object val = extract_header( &result_data, header_name);
        
        if( STR_Equals( "op", header_name) && val.type == MSGPACK_OBJECT_STR )
        {
            msgpack_object_str string = val.via.str;
            int str_len = string.size;
            char* str = Alloc(str_len);
            
            memset( str, '\0', str_len);
            str[str_len] = '\0';
            strncpy(str, string.ptr,str_len); 
            return str;  // dnagling pointer
        }

        return_status = msgpack_unpack_next(&unpacked_result, packet->buffer, packet->len, &off);

    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        PRINT("The data in the buf is invalid format.\n");
    }
}

void printKeyValuePair( Node* LinkedListNode)
{
	struct KeyValuePair* header = (struct KeyValuePair*) LinkedListNode->data;
	PRINT("key: %s, value: %s\n", header->key, header->value);
}

void printSetting( Node* LinkedListNode)
{
	struct KeyValuePair* header = (struct KeyValuePair*) LinkedListNode->data;
	PRINT("header: %s\n",header->key);
	List* settings = header->value;
	settings->fnPrint = printKeyValuePair;
	LIST_Print(settings);

}
