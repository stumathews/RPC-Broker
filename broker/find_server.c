#include "broker_support.h"
#include "common.h"

extern char port[20];
extern bool verbose;
extern bool waitIndef;
extern struct ServiceRegistration service_repository;

// find the registered server that has the service that the client has requested.
void find_server(char* buffer, int buflen, Destination *dest)
{
    dest->address = NULL;
    dest->port = NULL;

    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&result);

    ret = msgpack_unpack_next(&result, buffer, buflen, &off);

    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;
        
        char header_name[20];
        memset(header_name, '\0', 20);
        
        msgpack_object val = extract_header( &obj, header_name);
        
        if( STR_Equals( "op", header_name) && val.type == MSGPACK_OBJECT_STR )
        {
            msgpack_object_str string = val.via.str;
            // EXTRACT STRING START
            int str_len = string.size;
            char* str = Alloc( str_len);
            memset( str, '\0', str_len);
            str[str_len] = '\0';
            strncpy(str, string.ptr,str_len); 

            if( verbose )
                PRINT("Looking for %s\n", str);

            struct list_head *pos, *q;
            struct ServiceRegistration* tmp = malloc( sizeof( struct ServiceRegistration ));
            int count = 0;
        
            if( list_empty( &service_repository.list ))
            {
                PRINT("No services registered in broker.\n");
                return;
            }
            list_for_each( pos, &service_repository.list)
            {
                tmp = list_entry( pos, struct ServiceRegistration, list );
                ServiceReg *sreg = tmp;;
                bool found = false;

                if(verbose)
                    PRINT("Current SR is %s\n", sreg->service_name);
                for( int i = 0 ; i < sreg->num_services;i++)
                {
                    if( verbose )
                        PRINT("is %s == %s\n",str,sreg->services[i]);
                    if( STR_Equals( str, sreg->services[i]))
                    {
                        dest->address = sreg->address;
                        dest->port = sreg->port;
                        found = true; 
                        if(verbose)
                            PRINT("FOUND service for required service %s at %s:%s\n",str, dest->address,dest->port);
                        goto done;
                    }
                }
            }
        }

        ret = msgpack_unpack_next(&result, buffer, buflen, &off);

    } // finished unpacking.
done:
    if(verbose)
        PRINT("finished.\n");

    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        printf("The data in the buf is invalid format.\n");
    }
}
