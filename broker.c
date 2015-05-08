#include <stulibc.h>
#include "common.h"

char *program_name;
static char port[20] = {0};

static void server( SOCKET s, struct sockaddr_in *peerp )
{
    // read and process data on the socket.
    struct packet pkt;
    int n_rc = netReadn( s,(char*) &pkt.len, sizeof(uint32_t));
    pkt.len = ntohl(pkt.len);

    PRINT("received %d bytes and interpreted it as length of %u\n", n_rc,pkt.len );
    if( n_rc < 1 )
        netError(1, errno,"failed to receiver packet size\n");
    
    char* dbuf = (char*) malloc( sizeof(char) * pkt.len);
    int d_rc  = netReadn( s, dbuf, sizeof( char) * pkt.len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    printf("read %d bytes of data\n",d_rc);
    PRINT("Successfully received buffer:\n");
    unpack( (const char*)dbuf,pkt.len);


}
static void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( port, arg, strlen(arg));
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
    CMD_AddArgument(portNumber);

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

    if( argc > 1 )
    {
        enum ParseResult result = CMD_Parse(argc,argv,true);
        if( result != PARSE_SUCCESS )
        {
            PRINT("There was a problem parsing: %d \n", result);
            return 1;
        }
    }
    else
    {
        CMD_ShowUsages("broker <options>");
        exit(0);
    }

    PRINT("Broker starting.\n");

    INIT();

    // get a socket, bound to this address thats configured to listen.
    // NB: This is always ever non-blocking 
    s = netTcpServer("localhost",port);

    FD_SET(s, &readfds);

    do
    {
        PRINT("Listening.\n");
        // wait/block on this listening socket...
        int res = select( s+1, &readfds, NULL, NULL, &timeout);

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

    LIB_Uninit();
    EXIT( 0 );
}
