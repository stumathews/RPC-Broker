#include <stulibc.h>
#include "common.h"

struct ServiceRegistration service_repository;
static char port[20] = {0};
static bool verbose = false;
static bool waitIndef = false;
static void update_repository();
static void register_service(struct ServiceRegistration* service_registration);
static void UnpackServiceRegistrationBuffer(char* buffer,int buflen, struct ServiceRegistration* unpacked);
static void acknowledgement();
static void find_server(char* buffer, int len, Destination *dest);
static void find_client(char* buffer, int len, Destination *dest);
static void forward_request(char* buffer, int len);
static void forward_response();
static void setPortNumber(char* arg);
static void setVerbose(char* arg);
static void setWaitIndefinitely(char* arg);
static void main_event_loop();



// start broker and register command-line args
int main( int argc, char **argv )
{
    LIB_Init();
    INIT_LIST_HEAD(&service_repository.list);;

    struct Argument* portNumber = CMD_CreateNewArgument("port",
                                                        "port <number>",
                                                        "Set the port that the broker will listen on",
                                                        true,
                                                        true,
                                                        setPortNumber);
    struct Argument* verboseArg = CMD_CreateNewArgument("verbose",
                                                        "",
                                                        "Prints all messages verbosly",
                                                        false,
                                                        false,
                                                        setVerbose);
    struct Argument* waitIndefArg = CMD_CreateNewArgument("waitindef",
                                                        "",
                                                        "Wait indefinitely for new connections,else 60 secs and then dies",
                                                        false,
                                                        false,
                                                        setWaitIndefinitely);
    CMD_AddArgument(waitIndefArg);
    CMD_AddArgument(portNumber);
    CMD_AddArgument(verboseArg);


    if( argc > 1 )
    {
        enum ParseResult result = CMD_Parse(argc,argv,true);
        if( result != PARSE_SUCCESS )
        {
            return 1;
        }
    }
    else
    {
        CMD_ShowUsages("broker <options>");
        exit(0);
    }

    if(verbose) PRINT("Broker starting.\n");

    INIT();
    
    main_event_loop();

    LIB_Uninit();
    EXIT( 0 );
}

// ===============================
// Comand line processing routines
// ===============================

static void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( port, arg, strlen(arg));
}

static void setVerbose(char* arg)
{
    verbose = true;
}

static void setWaitIndefinitely(char* arg)
{
    waitIndef = true;
}

// ====================================
// Broker connection processing routine
// ====================================
// What we do with connections received from clients and servers who contact this broker.
static void server( SOCKET s, struct sockaddr_in *peerp )
{
    // 1. Read the size of packet.
    // 2. Read the packet data
    // 3. Do stuff based on the packet data

    struct packet pkt;

    int n_rc = netReadn( s,(char*) &pkt.len, sizeof(uint32_t));
    pkt.len = ntohl(pkt.len);

    if( n_rc < 1 )
        netError(1, errno,"Failed to receiver packet size\n");
    
    if(verbose) 
        PRINT("Received %d bytes and interpreted it as length of %u\n", n_rc,pkt.len );
    
    pkt.buffer = (char*) malloc( sizeof(char) * pkt.len);
    int d_rc  = netReadn( s, pkt.buffer, sizeof( char) * pkt.len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    
    if(verbose)
    {
        PRINT("Read %d bytes of data.\n",d_rc);
    }

    // do stuff based on the packet data
    int request_type = -1;
   
    if( (request_type = determine_request_type(&pkt)) == REQUEST_SERVICE )
    {
        forward_request(pkt.buffer, pkt.len);
    }
    else if ( request_type == REQUEST_REGISTRATION )
    {
        struct ServiceRegistration *sr_buf = Alloc( sizeof( struct ServiceRegistration ));
        // registration request will be put into sr_buf        
        UnpackServiceRegistrationBuffer(pkt.buffer, pkt.len,sr_buf); 
        register_service(sr_buf);
    }
    else if( request_type == REQUEST_SERVICE_RESPONSE )
    {
        Destination *dest = Alloc( sizeof( Destination ));
        find_client(pkt.buffer, pkt.len, dest); // the socket is already connected to the client if this is synchrnous
        forward_response(); //to the client
    }
    else 
    {
        PRINT("Unrecongnised request type:%di. Ignoring \n", request_type);    
        return;
    }

}


// waits for service registrations from services and service requests from clients.
// Forwards client requests to servers, processes service requetss from servers
// sends service replies to clients
static void main_event_loop()
{
    struct sockaddr_in local;
    struct sockaddr_in peer;
    
    char *hname;
    char *sname;
    int peerlen;
    
    SOCKET s1;
    SOCKET s;
    
    fd_set readfds;
    FD_ZERO( &readfds);
    const int on = 1;
    struct timeval timeout = {.tv_sec = 60, .tv_usec=0}; 

    // get a socket, bound to this address thats configured to listen.
    // NB: This is always ever non-blocking 
    s = netTcpServer("localhost",port);

    FD_SET(s, &readfds);

    do
    {
       if(verbose) PRINT("Listening.\n");
       // wait/block on this listening socket...
       int res = 0;

       if( waitIndef )
          res =  select( s+1, &readfds, NULL, NULL, NULL);//&timeout);
       else
          res =  select( s+1, &readfds, NULL, NULL, &timeout);

        if( res == 0 )
        {
            LOG( "timeout");
            netError(1,errno,"timeout!");
        }
        else if( res == -1 )
        {
            LOG("Select error!");
            netError(1,errno,"select error!!");
        }
        else
        {
            peerlen = sizeof( peer );

            if( FD_ISSET(s,&readfds ))
            {
                s1 = accept( s, ( struct sockaddr * )&peer, &peerlen );

                if ( !isvalidsock( s1 ) )
                    netError( 1, errno, "accept failed" );
                
                //Process connection:
                server( s1, &peer );
                CLOSE( s1 );
            }
            else
            {
                DBG("Not our socket. continuing");
                continue;
            }
        }
    } while ( 1 );
}

static void update_repository()
{
    
}

static void print_service_repository()
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

// Unpack the service registration request.
// Populate the provided ServiceRegistration buffer parameter
static void UnpackServiceRegistrationBuffer(char* buffer, int buflen, struct ServiceRegistration* unpacked)
{
    if( verbose)
        PRINT("Unpacking service registration request...\n");

    unpacked->num_services = 0; // set this to 0 so we know if its set to something else later or not

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

        if( val.type == MSGPACK_OBJECT_STR )
        {
            // EXTRACT STRING START
            int str_len = val.via.str.size;
            char* str = Alloc( str_len);
            memset( str, '\0', str_len);
            str[str_len] = '\0';
            strncpy(str, val.via.str.ptr,str_len); 
            // EXTRACT STRING END 
    
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
            if( verbose) 
                PRINT("Processing services...\n");

            msgpack_object_array array = val.via.array;
            for( int i = 0; i < array.size;i++)
            {
                // EXTRACT STRING START
                int str_len = array.ptr[i].via.str.size;
                char* str = Alloc( str_len);
                memset( str, '\0', str_len);
                str[str_len] = '\0';
                strncpy(str, array.ptr[i].via.str.ptr,str_len); 
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
} // UnpackRegistrationBuffer

// Add the service registration request to the service repository
static void register_service(struct ServiceRegistration* service_registration )
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
    
    list_add( &(service_registration->list),&(service_repository.list));
    
    if( verbose )
        print_service_repository(); // prints all services registered so far

}

static void acknowledgement()
{
    // Send a message back to sender(client or server) with general ACK
}

// find the registered server that has the service that the client has requested.
static void find_server(char* buffer, int buflen, Destination *dest)
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

static void find_client(char *buffer, int len, Destination *dest)
{
}

// send the client's service requets to the server that is known to be able to process it
static void forward_request(char* buffer, int len)
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
static void forward_response()
{
}
