#include <msgpack.h>
#include <stulibc.h>
#include "common.h"

extern bool wait_response_indef;
static struct packet *process_response( SOCKET s, struct sockaddr_in *peerp );
extern bool verbose;

struct packet *send_and_receive(char* buffer, int bufsize,char* address, char* port, bool verbose, char* wait_response_port)
{
    send_request(buffer, bufsize, address, port, verbose );

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
    struct packet *result;
    struct timeval timeout = {.tv_sec = 60, .tv_usec=0}; 

    INIT();
    
    if( verbose ) 
        PRINT("client waiting for response...\n");

    // get a socket, bound to this address thats configured to listen.
    // NB: This is always ever non-blocking 
    s = netTcpServer("localhost",wait_response_port);

    FD_SET(s, &readfds);
    if(verbose) PRINT("About wait for read on port %s...\n", wait_response_port);
        // wait/block on this listening socket...
        int res = 0;
        if( !wait_response_indef )
            res = select( s+1, &readfds, NULL, NULL, &timeout);
        else
            res = select( s+1, &readfds, NULL, NULL, NULL);

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
                result = process_response( s1, &peer );
                CLOSE( s1 );
            }
            else
            {
                DBG("not our socket. continuing");
            }
        }
    CLOSE(s);
    return result;
}

struct packet *process_response( SOCKET s, struct sockaddr_in *peerp )
{

    // Wait for connections from the broker.
    struct packet *pkt = Alloc( sizeof( struct packet));
    int n_rc = netReadn( s,(char*) &pkt->len, sizeof(uint32_t));
    pkt->len = ntohl(pkt->len);

    if( verbose)
        PRINT("received %d bytes and interpreted it as length of %u\n", n_rc,pkt->len );

    if( n_rc < 1 )
        netError(1, errno,"failed to receiver packet size\n");
    
    pkt->buffer = (char*) Alloc( sizeof(char) * pkt->len);
    int d_rc  = netReadn( s, pkt->buffer, sizeof( char) * pkt->len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    
    if(verbose)
    {
        PRINT("read %d bytes of data\n",d_rc);
    }
    return pkt;
}
