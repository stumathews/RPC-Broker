#include <stulibc.h>

char *program_name;
static char* port = {0};

static void server( SOCKET s, struct sockaddr_in *peerp )
{
    // read and process data on the socket.
}
static void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    int arglen = strlen(arg);
    
    port = Alloc( sizeof( char) * arglen );
    strncpy( port, arg, arglen);
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
