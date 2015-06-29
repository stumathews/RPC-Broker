#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern bool verbose_flag;
extern struct ServiceRegistration service_repository;

struct ServiceRegistration* unpack_service_registration_buffer(char* buffer, int buflen)
{
    if( verbose_flag)
        PRINT("Unpacking service registration request...\n");

    struct ServiceRegistration* unpacked = Alloc( sizeof( struct ServiceRegistration) );
    unpacked->num_services = 0; // set this to 0 so we know if its set to something else later or not

    size_t off = 0;
    int i = 0;
    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    msgpack_unpacked_init(&unpacked_result);

    return_status = msgpack_unpack_next(&unpacked_result, buffer, buflen, &off);

    while (return_status == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = unpacked_result.data;

        char header_name[MAX_HEADER_NAME_SIZE];
        memset(header_name, '\0', MAX_HEADER_NAME_SIZE);

        msgpack_object val = extract_header( &obj, header_name);

        if( val.type == MSGPACK_OBJECT_STR )
        {
            int str_len = val.via.str.size;
            char* str = Alloc( str_len);

            memset( str, '\0', str_len);
            str[str_len] = '\0';
            strncpy(str, val.via.str.ptr,str_len); 
    
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
        }
        else if(val.type == MSGPACK_OBJECT_POSITIVE_INTEGER)
        {
            if( STR_Equals("services-count",header_name) == true)
            {
                unpacked->num_services = val.via.i64;
                unpacked->services = Alloc(sizeof(char)*val.via.i64);
            }
        }
        else if( val.type == MSGPACK_OBJECT_ARRAY )
        {
            if( verbose_flag) 
                PRINT("Processing services...\n");

            msgpack_object_array array = val.via.array;

            for( int i = 0; i < array.size;i++)
            {
                struct msgpack_object curr = array.ptr[i];
                int str_len = curr.via.str.size;
                char* str = Alloc( str_len); 
                
                memset( str, '\0', str_len);
                str[str_len] = '\0';
                strncpy(str, curr.via.str.ptr,str_len); 

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
        return_status = msgpack_unpack_next(&unpacked_result, buffer, buflen, &off);
    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        printf("The data in the buf is invalid format.\n");
    }

    return unpacked;
} 
