#include <stulibc.h>
#include "common.h"


// forward declerations
static void server( SOCKET s, struct sockaddr_in *peerp );

char *program_name;
static char port[20] = {0};
static bool verbose = false;
static bool waitIndef = false;

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

void register_service()
{
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

enum RequestType determine_request_type(struct packet* pkt)
{
        
    /* buf is allocated by client. */
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    msgpack_unpacked_init(&result);

    // Go ahead unpack an object
    ret = msgpack_unpack_next(&result, pkt->buffer, pkt->len, &off);
    if (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;

        msgpack_object_print(stdout, obj);

        if( obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER || obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER )
        {            
            return obj.via.u64;
        }
        else
        {
            PRINT("Expected reqyest type in protocol.\n");
            exit(1);
        }
    }

    msgpack_unpacked_destroy(&result);
}

static void server( SOCKET s, struct sockaddr_in *peerp )
{
    // read and process data on the socket.
    struct packet pkt;
    int n_rc = netReadn( s,(char*) &pkt.len, sizeof(uint32_t));
    pkt.len = ntohl(pkt.len);

    if( verbose) PRINT("received %d bytes and interpreted it as length of %u\n", n_rc,pkt.len );
    if( n_rc < 1 )
        netError(1, errno,"failed to receiver packet size\n");
    
    pkt.buffer = (char*) malloc( sizeof(char) * pkt.len);
    int d_rc  = netReadn( s, pkt.buffer, sizeof( char) * pkt.len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    
    if(verbose)
    {
        printf("read %d bytes of data\n",d_rc);
        PRINT("Successfully received buffer:");
    }

    // What is this data we got?
    // 1. A request for a service
    // 2. A registration request
    int request_type = determine_request_type(&pkt);
    if( request_type == REQUEST_SERVICE )
    {
        PRINT("Incomming Service Request.\n");
        find_server();
        forward_request();
    }
    else if ( request_type == REQUEST_REGISTRATION )
    {
        PRINT("Incomming Registration Request.\n");

    }
    else 
    {
        PRINT("Unrecongnised request type:%d \n", request_type);    
        exit(1);
    }


    find_client(); // the socket is already connected to the client if this is synchrnous
    forward_response();
   
    // in theory this is a client side operation not broker. broker just forward to registered servers 
    unpack_request_data( (const char*)pkt.buffer,pkt.len);


}

int main( int argc, char **argv )
{
    LIB_Init();

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
