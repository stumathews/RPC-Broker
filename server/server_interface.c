/* server-proxy.c
 * Acts as the main service and calls the server functions.
 * 
 * Responsibility: Abstract communication with broker away from server functions.
 * 
 * */
#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

#define CONFIG_FILENAME "config.ini"
/**
 * @brief the server port 
 * 
 */
char port[MAX_PORT_CHARS] = {0};
/**
 * @brief the brokers port
 * 
 */
char broker_port[MAX_PORT_CHARS] = {0};
/**
 * @brief the brokers address
 * 
 */
char broker_address[MAX_ADDRESS_CHARS] = {0};
/**
 * @brief the servers address
 * 
 */
char server_address[MAX_ADDRESS_CHARS] = {0};
/**
 * @brief wait forever for data on socket?
 * 
 */
static bool waitIndef = false;
/**
 * @brief Should we say everything we do 
 * 
 */
bool verbose = false;
/**
 * @brief Is registered with broker
 * 
 */
static bool registered_with_broker = false;
/**
 * @brief Register with the broker_
 * 
 * @param broker_address the broker address to contact for registration
 * @param broker_port the broker port
 * @return bool
 */
bool service_register_with_broker( char *broker_address, char* broker_port );
/**
 * @brief Unpack service request, execute operation adn send the response back to the broker
 * 
 * @param buffer the client's request
 * @param buflen the length of the request
 * @return void
 */
void unpack_marshal_call_send( char* buffer, int buflen);

static void setBrokerPort( char* arg);
static void setPortNumber(char* arg);
static void setBrokerAddress(char* arg);
static void setOurAddress(char* arg);
static void setWaitIndef(char* arg);
static void setBeVerbose(char* arg);
/**
 * @brief Main Server processing routine. called when the server gets a connection from the broker
 * 
 * @param s the socket
 * @param peerp the peerp
 * @return void
 */
static void server( SOCKET s, struct sockaddr_in *peerp );


int main( int argc, char **argv )
{
    LIB_Init();
    PRINT("Server starting...");

    struct Argument* portNumber = 		CMD_CreateNewArgument("p","p <number>","Set the port that the server will listen on", true, true, setPortNumber);
    struct Argument* brokerAddressCMD = CMD_CreateNewArgument("ba","ba <address>","Set the address of the broker", true, true, setBrokerAddress);
    struct Argument* ourAddressCMD = 	CMD_CreateNewArgument("oa","oa <address>","Set the address we listen on (broker uses to deliver responses)", true, true, setOurAddress);
    struct Argument* brokerPortCMD = 	CMD_CreateNewArgument("bp","bp <port>","Set the port of the broker", true, true, setBrokerPort);
    struct Argument* waitIndefCMD = 	CMD_CreateNewArgument("w","","wait indefinitely for connections", false, false, setWaitIndef);
    struct Argument* beVerboseCMD = 	CMD_CreateNewArgument("v","","be verbose wtih messages", false, false, setBeVerbose);

    CMD_AddArgument(portNumber);
    CMD_AddArgument(waitIndefCMD);
    CMD_AddArgument(brokerAddressCMD);
    CMD_AddArgument(brokerPortCMD);
    CMD_AddArgument(beVerboseCMD);
    CMD_AddArgument(ourAddressCMD);

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
    List* settings = (void*)null;

    struct timeval timeout = {.tv_sec = 60, .tv_usec=0}; 

    if (FILE_Exists(CONFIG_FILENAME) && !(argc > 1))
	{
		DBG("Using config file located in '%s'", CONFIG_FILENAME);
		settings = LIST_GetInstance();
		if(INI_IniParse(CONFIG_FILENAME, settings) == 0) // if successful parse
		{
			setWaitIndef(INI_GetSetting(settings, "options", "wait"));
			setBeVerbose(INI_GetSetting(settings, "options", "verbose"));
			setPortNumber(INI_GetSetting(settings, "networking", "port"));
			setOurAddress(INI_GetSetting(settings, "networking", "address"));

			setBrokerAddress(INI_GetSetting(settings, "broker", "address"));
			setBrokerPort(INI_GetSetting(settings, "broker", "port"));

			if(verbose)
			{
				LIST_ForEach(settings, printSetting);
			}
		}
		else
		{
			ERR_Print("Failed to parse config file", 1);
		}
	}
    else if( argc > 1 )
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
        CMD_ShowUsages("server", "stumathews@gmail.com", "the server component");
        exit(0);
    }

    NETINIT();
    
    if( verbose ) 
        PRINT("Server listening...\n");

    // Register with the broker on startup. Currently dont wit for an ACK
    if(service_register_with_broker(broker_address, broker_port)){
    	PRINT("Sending blind registration request to broker at address '%s:%s'", broker_address, broker_port);
        registered_with_broker = true;
    }

    // get a socket, bound to this address thats configured to listen.
    // NB: This is always ever non-blocking 
    s = netTcpServer(broker_address,port);

    FD_SET(s, &readfds);
    if(verbose) PRINT("About wait for read on port %s...\n", port);
    do
    {
        // wait/block on this listening socket...
        int res = 0;
        if( !waitIndef )
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
                server( s1, &peer );
                NETCLOSE( s1 );
            }
            else
            {
                DBG("not our socket. continuing");
                continue;
            }
        }

    } while ( 1 );

    LIST_FreeInstance(settings);
    LIB_Uninit();
    EXIT( 0 );
}


static void server( SOCKET s, struct sockaddr_in *peerp )
{

    // Wait for connections from the broker.
    Packet pkt;
    int n_rc = netReadn( s,(char*) &pkt.len, sizeof(uint32_t));
    pkt.len = ntohl(pkt.len);

    if( verbose)
        PRINT("received %d bytes and interpreted it as length of %u\n", n_rc,pkt.len );

    if( n_rc < 1 )
        netError(1, errno,"failed to receiver packet size\n");
    
    char* dbuf = (char*) malloc( sizeof(char) * pkt.len);
    int d_rc  = netReadn( s, dbuf, sizeof( char) * pkt.len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    
    if(verbose)
    {
        PRINT("read %d bytes of data\n",d_rc);
    }
    unpack_marshal_call_send( dbuf, pkt.len);

}
// ===============================
// Command line handling routines
// ===============================
static void setBrokerPort( char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( broker_port, arg, strlen(arg));
}

static void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( port, arg, strlen(arg));
}

static void setBrokerAddress(char* arg)
{
    CHECK_STRING( arg, IS_NOT_EMPTY );
    strncpy( broker_address, arg, strlen(arg) );
}
static void setOurAddress(char* arg)
{
    CHECK_STRING( arg, IS_NOT_EMPTY );
    strncpy( server_address, arg, strlen(arg) );
}

static void setWaitIndef(char* arg)
{
    waitIndef = true;
}

static void setBeVerbose(char* arg)
{
    if( STR_Equals(arg,"true") || STR_Equals(arg,"1")) {
	verbose = true;
    } else {

	verbose = false;
    }
}
