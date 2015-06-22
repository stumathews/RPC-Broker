#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern bool verbose_flag;
extern struct ServiceRegistration service_repository;

struct ServiceRegistration* UnpackServiceRegistrationBuffer(char* buffer, int buflen)
{
    if( verbose_flag)
        PRINT("Unpacking service registration request...\n");

    struct ServiceRegistration* unpacked = Alloc( sizeof( struct ServiceRegistration) );
    unpacked->num_services = 0; // set this to 0 so we know if its set to something else later or not

    size_t off = 0;
    int i = 0;
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    msgpack_unpacked_init(&result);

    ret = msgpack_unpack_next(&result, buffer, buflen, &off);

    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        char header_name[20]; // protocol specification states that header name will be no longer than 20 characters
        msgpack_object obj = result.data;
        memset(header_name, '\0', 20);
        msgpack_object val = extract_header( &obj, header_name);

        if( val.type == MSGPACK_OBJECT_STR )
        {
            // EXTRACT STRING START
            int str_len = val.via.str.size;
            char* str = Alloc( str_len);
            memset( str, '\0', str_len);
            str[str_len] = '\0';
            strncpy(str, val.via.str.ptr,str_len); 
            // EXTRACT STRING END 
    
            // PICK OUT PROTOCOL HEADERS - string values
            if( STR_Equals( "sender-address", header_name ) == true)
            {
                unpacked->address = str;
            }
            else if( STR_Equals("reply-port",header_name) == true)
            {
                unpacked->port = str;
            }
            else if( STR_Equals("service-name",header_name) == true)
            {
                unpacked->service_name = str;
            }
            // PICK OUT PROTOCOL HEADERS
        }
        else if(val.type == MSGPACK_OBJECT_POSITIVE_INTEGER)
        {
            // PICK OUT INT HEADERS
            if( STR_Equals("services-count",header_name) == true)
            {
                unpacked->num_services = val.via.i64;
                unpacked->services = Alloc(sizeof(char)*val.via.i64);
            }
        }
        else if( val.type == MSGPACK_OBJECT_ARRAY )
        {
            // PICK OUT ARRAY HEADERS
            if( verbose_flag) 
                PRINT("Processing services...\n");

            msgpack_object_array array = val.via.array;
            for( int i = 0; i < array.size;i++)
            {
                struct msgpack_object curr = array.ptr[i];
                int str_len = curr.via.str.size;
                char* str = Alloc( str_len); // new string
                
                // EXTRACT STRING START
                memset( str, '\0', str_len);
                str[str_len] = '\0';
                strncpy(str, curr.via.str.ptr,str_len); 
                // EXTRACT STRING END 

                unpacked->services[i] = str;

                if(verbose_flag)
                    PRINT("Found service: '%s'\n",str);
            }

        } //array processing end
        else
        {
            // this is not a header or array but something else
            printf("\n"); 
        }
        ret = msgpack_unpack_next(&result, buffer, buflen, &off);
    } // finished unpacking.

    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        printf("The data in the buf is invalid format.\n");
    }
    return unpacked;
} 
