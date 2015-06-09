#include "broker_support.h"
#include "common.h"

extern char port[20];
extern bool verbose;
extern bool waitIndef;
extern struct ServiceRegistration service_repository;

// ===============================
// Comand line processing routines
// ===============================

void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( port, arg, strlen(arg));
}

void setVerbose(char* arg)
{
    verbose = true;
}

void setWaitIndefinitely(char* arg)
{
    waitIndef = true;
}
void print_service_repository()
{
    if(verbose)
        printf("Service registrations:\n");

    struct list_head *pos, *q;
    struct ServiceRegistration* tmp = Alloc( sizeof( struct ServiceRegistration ));
    int count = 0;

    list_for_each( pos, &service_repository.list)
    {
        tmp = list_entry( pos, struct ServiceRegistration, list );
        if( tmp  == NULL )
        {
            PRINT("Null service!\n");
            return;
        }

        if( verbose )
            PRINT("In list_for_each\n");

        PRINT("Service Registration:\n"
                "Service name:%s\n"
                "Address: %s\n"
                "Port: %s\n"
                "Number ofservices %d",tmp->service_name,tmp->address, tmp->port,tmp->num_services);
    }
}

// Unpack the service registration request, return it in ServiceRegistration to caller
void UnpackServiceRegistrationBuffer(char* buffer, int buflen, struct ServiceRegistration* unpacked)
{
    // NB: struct ServiceRegistration* unpacked will be populated
    // --

    if( verbose)
        PRINT("Unpacking service registration request...\n");

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
            if( verbose) 
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

                if(verbose)
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
} 

// Add the service registration request to the service repository
void register_service(struct ServiceRegistration* service_registration )
{
    if(verbose)
        PRINT("Registering service '%s':\n",service_registration->service_name);

    if( verbose)
    {
        for( int i = 0 ; i < service_registration->num_services;i++)
        {
            PRINT("Service %s\n", service_registration->services[i]);
        }
    }
    
    list_add( &(service_registration->list),&(service_repository.list)); // add service registration to the repository
    
    if( verbose )
        print_service_repository();

}


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

void find_client(char *buffer, int len, Destination *dest)
{
}

// send the client's service requets to the server that is known to be able to process it
void forward_request(char* buffer, int len)
{
    Destination *dest = Alloc( sizeof( Destination ));
    find_server(buffer, len, dest );

    if( dest->address == NULL ||  dest->port == NULL ) 
    {
        if(verbose)
            PRINT("No server can process that request\n");
        return;
    }
    
    if(verbose) 
        PRINT("About to send request to service at %s:%s\n", dest->address, dest->port);

    send_request( buffer, len, dest->address, dest->port,verbose);
}

// When the broker gets a response form the server, it will need to send it back to the originting client that requeted it.
void forward_response()
{

}

void update_repository()
{
    
}

void acknowledgement()
{
    // Send a message back to sender(client or server) with general ACK
}
