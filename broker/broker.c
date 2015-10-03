#include <stulibc.h>
#include "broker_support.h"

struct ServiceRegistration service_repository;
struct ClientRequestRegistration client_request_repository;
char port[MAX_PORT_CHARS] = {0};
bool verbose_flag = false;
bool waitIndef_flag = false;
char our_address[MAX_ADDRESS_CHARS] = {0};

static void main_event_loop();
static void server( SOCKET s, struct sockaddr_in *peerp );


/**
 * @brief see broker_support.c for additional functions used in the broker code
 * 
 * @param argc Number of args
 * @param argv args
 * @return int
 */
int main( int argc, char **argv )
{
    LIB_Init();

    INIT_LIST_HEAD(&service_repository.list);
    INIT_LIST_HEAD(&client_request_repository.list);

    struct Argument* portNumberArg = CMD_CreateNewArgument("port",
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
                                                        setVerboseFlag);

    struct Argument* waitIndefArg = CMD_CreateNewArgument("waitindef",
                                                        "",
                                                        "Wait indefinitely for new connections, else 60 secs and then dies",
                                                        false,
                                                        false,
                                                        setWaitIndefinitelyFlag);

    struct Argument* ourAddressCMD = CMD_CreateNewArgument("our-address",
                                                        "our-address <address>",
                                                        "Set our address",
                                                        true,
                                                        true,
                                                        setOurAddress);

    CMD_AddArgument(waitIndefArg);
    CMD_AddArgument(portNumberArg);
    CMD_AddArgument(verboseArg);
    CMD_AddArgument(ourAddressCMD);


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
        CMD_ShowUsages("broker","stumathews@gmail.com","a broker component");
        exit(0);
    }

    if(verbose_flag) PRINT("Broker starting.\n");

    INIT();
    
    main_event_loop();

    LIB_Uninit();
    EXIT( 0 );
}


/**
 * @brief Continually waits for network service registrations from services,  and service requests from clients.
 * 
 * @return void
 */
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

    // NB: This is always ever non-blocking 
    s = netTcpServer(our_address,port);

    FD_SET(s, &readfds);

    do
    {
       if(verbose_flag) PRINT("** Listening.\n");

       // wait/block on this listening socket...
       int res = 0;

       if( waitIndef_flag )
          res =  select( s+1, &readfds, NULL, NULL, NULL);
       else
          res =  select( s+1, &readfds, NULL, NULL, &timeout);

        if( res == 0 )
        {
            LOG( "broker listen timeout!");
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
                
                //Data arrived: Process it
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


/**
 * @brief Broker connection processing routine. Deals with connections received from clients and servers.
 * 
 * @param s The socket
 * @param peerp the peerlen
 * @return void
 */
static void server( SOCKET s, struct sockaddr_in *peerp )
{
    // 1. Read the size of packet.
    // 2. Read the packet data
    // 3. Do stuff based on the packet data

    struct Packet packet;

    int n_rc = netReadn( s,(char*) &packet.len, sizeof(uint32_t));
    packet.len = ntohl(packet.len);

    if( n_rc < 1 ) netError(1, errno,"Failed to receiver packet size\n");
    if( verbose_flag ) PRINT("Received %d bytes and interpreted it as length of %u\n", n_rc,packet.len );
    
    packet.buffer = (char*) Alloc( sizeof(char) * packet.len);
    int d_rc  = netReadn( s, packet.buffer, sizeof( char) * packet.len);

    if( d_rc < 1 )  netError(1, errno,"failed to receive message\n");
    if(verbose_flag) PRINT("Read %d bytes of data.\n",d_rc);

    int request_type = -1; // -1 represents invalid state
   
    if( (request_type = determine_request_type(&packet)) == REQUEST_SERVICE )
    {
        Location *src = get_sender_address( &packet, peerp); 
        forward_request(&packet, src); // to the server
    } 
    else if ( request_type == REQUEST_REGISTRATION )  
    {
        register_service_request(&packet);
    } 
    else if( request_type == REQUEST_SERVICE_RESPONSE )
    {
        Packet* response = &packet;
        forward_response(response); //to the client
    } 
    else 
    {
        PRINT("Unrecongnised request type:%d. Ignoring \n", request_type);    
    }

    MEM_DeAlloc( packet.buffer, "packet.buffer" );
    
    return;
}
