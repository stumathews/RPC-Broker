#include <stulibc.h>
#include "common.h"

struct ServiceRegistrationsList service_repository;
char *program_name;
static char port[20] = {0};
static bool verbose = false;
static bool waitIndef = false;

void update_repository();
void register_service(struct ServiceRegistration* service_registration);
void UnpackServiceRegistrationBuffer(char* buffer,int buflen, struct ServiceRegistration* unpacked);
void acknowledgement();
void find_server();
void find_client();
void forward_request();
void forward_response();

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
    
    goto test;
    
    // BrokerProtocol* 

    // What is this data we got?
    int request_type = 0;
    if( (request_type = determine_request_type(&pkt)) == REQUEST_SERVICE )
    {
        PRINT("Incomming Service Request.\n");
        find_server();
        forward_request();
    }
    else if ( request_type == REQUEST_REGISTRATION )
    {
        test:
        if( verbose )
            PRINT("Incomming Registration Request.\n");

        struct ServiceRegistration *sr_buf = malloc( sizeof( struct ServiceRegistration ));
        UnpackServiceRegistrationBuffer(pkt.buffer, pkt.len,sr_buf); 
        register_service(sr_buf);
        free(sr_buf);

    }
    else 
    {
        PRINT("Unrecongnised request type:%d \n", request_type);    
        exit(1);
    }


    find_client(); // the socket is already connected to the client if this is synchrnous
    forward_response();
   


}

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

// listens for broker connections and dispatches them to the server() method
void main_event_loop()
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

                // do network functionality on this socket that now represents a connection with the peer (client) 
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

void update_repository()
{
    
}
void print_service_repository()
{
    if(verbose)
        printf("Service registrations:\n");

    struct list_head *pos, *q;
    struct ServiceRegistrationsList* tmp = malloc( sizeof( struct ServiceRegistrationsList ));
    int count = 0;
    list_for_each( pos, &service_repository.list)
    {
        tmp = list_entry( pos, struct ServiceRegistrationsList, list );
        printf("%d: Address : %s\n Port %s\n", count++,tmp->service_registration->address, tmp->service_registration->port);
    }
    //free(tmp);
}

void UnpackServiceRegistrationBuffer(char* buffer, int buflen, struct ServiceRegistration* unpacked)
{
    // unpack service registration request
    if( verbose)
        PRINT("unpack service registration request\n");
    unpacked->address = "dummy";
    unpacked->port = "7070";
    // in theory this is a client side operation not broker. broker just forward to registered servers 
    unpack_request_data( (const char*)buffer,buflen);
}

void register_service(struct ServiceRegistration* service_registration )
{
    if(verbose)
        PRINT("register service registration request\n");

    struct ServiceRegistrationsList *tmp = malloc( sizeof( struct ServiceRegistrationsList));
    tmp->service_registration = service_registration;
    list_add( &(tmp->list),&(service_repository.list));
    
    print_service_repository(); // prints all services registered so far

}

void acknowledgement()
{
}

void find_server()
{
}

void find_client()
{
}

void forward_request()
{

}

void forward_response()
{
}



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
