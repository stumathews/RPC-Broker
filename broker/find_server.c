#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern struct ServiceRegistration service_repository;

static char* get_op_name( char* protocol_buffer, int protocol_buffer_len);

Destination* find_server(char* buffer, int buflen)
{
    char* op_name = get_op_name( buffer, buflen );

    Destination *dest = Alloc( sizeof(Destination) );
    dest->address = NULL;
    dest->port = NULL;


    struct list_head *pos, *q;

    if( list_empty( &service_repository.list ))
    {
        PRINT("No services registered in broker.\n");
    }

    list_for_each( pos, &service_repository.list)
    {
        ServiceReg *sreg_entry  = list_entry( pos, struct ServiceRegistration, list );

        for( int i = 0 ; i < sreg_entry->num_services;i++)
        {
            if( STR_Equals( op_name, sreg_entry->services[i]))
            {
                dest->address = sreg_entry->address;
                dest->port = sreg_entry->port;
                PRINT("FOUND server for required service '%s' at location '%s:%s'\n",op_name, dest->address,dest->port);
                return dest;
            }
        }
    }
    return dest;
}

static char* get_op_name( char* protocol_buffer, int protocol_buffer_len)
{

    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    size_t off = 0;
<<<<<<< HEAD
    char header_name[MAX_HEADER_NAME_SIZE] = {0};
    
    msgpack_unpacked_init(&unpacked_result);
=======
    msgpack_unpacked_init(&result);
>>>>>>> 9efbca1fef9a36263e37ece84be0f19e0260a3d3

    return_status = msgpack_unpack_next(&unpacked_result, protocol_buffer, protocol_buffer_len, &off);

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
<<<<<<< HEAD
            return str; 
=======

            struct list_head *pos, *q;
        
            if( list_empty( &service_repository.list ))
            {
                PRINT("No services registered in broker.\n");
                return;
            }

            list_for_each( pos, &service_repository.list)
            {
                ServiceReg *sreg  = list_entry( pos, struct ServiceRegistration, list );

                for( int i = 0 ; i < sreg->num_services;i++)
                {
                    if( STR_Equals( str, sreg->services[i]))
                    {
                        dest->address = sreg->address;
                        dest->port = sreg->port;
                        PRINT("FOUND server for required service '%s' at location '%s:%s'\n",str, dest->address,dest->port);
                        goto done;
                    }
                }
            }
>>>>>>> 9efbca1fef9a36263e37ece84be0f19e0260a3d3
        }

        return_status = msgpack_unpack_next(&unpacked_result, protocol_buffer, protocol_buffer_len, &off);

    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        printf("The data in the buf is invalid format.\n");
    }
}
