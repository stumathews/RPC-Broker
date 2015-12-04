#include <msgpack.h>
#include <stulibc.h>
#include "common.h"

extern bool wait_response_indef;
extern bool verbose;
extern char our_address[MAX_ADDRESS_CHARS];
static Packet *get_response( SOCKET s, struct sockaddr_in *peerp );

/**
 * @brief Sends a packet of data and waits for its response
 * 
 * @param packet the data to send
 * @param address the address to send the data to
 * @param port the port to connect to on the address
 * @param verbose true if this function should log verbose messages
 * @param wait_response_port the port that we'll listen locally on for responses(we send this port to recipient)
 * @return Packet* the response from the receipient
 */
Packet *send_and_receive(Packet* packet,char* address, char* port, bool verbose, char* wait_response_port)
{
    send_request(packet, address, port, verbose );

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
    Packet *result;
    struct timeval timeout = {.tv_sec = 60, .tv_usec=0}; 

    NETINIT();

    if( verbose ) 
        PRINT("client waiting for response...\n");

    // NB: This is always ever non-blocking 
    s = netTcpServer(our_address ,wait_response_port);

    FD_SET(s, &readfds);
    if(verbose) { PRINT("About wait for read on port %s...\n", wait_response_port); }

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
            result = get_response( s1, &peer );
            NETCLOSE( s1 );
        }
        else { DBG("not our socket. continuing"); }
    }
    NETCLOSE(s);
    return result; // dangling pointer - stack frame finishes and could invalidate this address
}


/**
 * @brief Read data from socket, and return it
 * 
 * @param s the socket we're reading from
 * @param peerp the peer on the other side
 * @return Packet* te response data we read from the socket
 */
Packet *get_response( SOCKET s, struct sockaddr_in *peerp )
{

    Packet *packet = Alloc( sizeof(Packet));
    
    int n_rc = netReadn( s,(char*) &packet->len, sizeof(uint32_t));
    packet->len = ntohl(packet->len);

    if( verbose)
        PRINT("received %d bytes and interpreted it as length of %u\n", n_rc,packet->len );

    if( n_rc < 1 )
        netError(1, errno,"failed to receiver packet size\n");
    
    packet->buffer = (char*) Alloc( sizeof(char) * packet->len);
    int d_rc  = netReadn( s, packet->buffer, sizeof( char) * packet->len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    
    if(verbose) { PRINT("read %d bytes of data\n",d_rc); }

    return packet; // dangling pointer - add might be deallocated when stack frame finishes
}
