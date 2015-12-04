#include <stulibc.h>
#include "broker_support.h"
#include "../config.h"

struct ServiceRegistration service_repository;
struct ClientRequestRegistration client_request_repository;
char port[MAX_PORT_CHARS] = {0};
char our_address[MAX_ADDRESS_CHARS] = {0};
static const char* CONFIG_FILENAME = "config.ini";
bool verbose_flag = false;
bool waitIndef_flag = false;


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

    struct Argument* cmdPort = 		 CMD_CreateNewArgument("p", "p <number>", "Set the port that the broker will listen on",true, true, setPortNumber);
    struct Argument* cmdVerbose =    CMD_CreateNewArgument("v","","Prints all messages verbosly",false,false,setVerboseFlag);
    struct Argument* cmdWaitIndef =  CMD_CreateNewArgument("w","","Wait indefinitely for new connections, else 60 secs and then dies",false,false,setWaitIndefinitelyFlag);
    struct Argument* cmdMyAddress =  CMD_CreateNewArgument("a","a <address>","Set our address",true, true, setOurAddress);

    List* settings = {0};

    CMD_AddArgument(cmdWaitIndef);
    CMD_AddArgument(cmdPort);
    CMD_AddArgument(cmdVerbose);
    CMD_AddArgument(cmdMyAddress);

    if (FILE_Exists(CONFIG_FILENAME) && !(argc > 1))
    {
    	DBG("Using config file located in '%s'", CONFIG_FILENAME);
    	settings = LIST_GetInstance();
    	if(INI_IniParse(CONFIG_FILENAME, settings) == 0) // if successful parse
    	{
    		setWaitIndefinitelyFlag(INI_GetSetting(settings, "options", "wait"));
    		setVerboseFlag(INI_GetSetting(settings, "options", "verbose"));
			setPortNumber(INI_GetSetting(settings, "networking", "port"));
			setOurAddress(INI_GetSetting(settings, "networking", "address"));

			if(verbose_flag)
			{
				LIST_ForEach(settings, printSetting);
			}
    	}
    	else
    	{
    		ERR_Print("Failed to parse config file", 1);
    	}
    }
    else if(argc > 1)
    {
    	if((CMD_Parse(argc, argv, true) != PARSE_SUCCESS))
		{
    	  PRINT("CMD line parsing failed.");
		  return 1;  // Note CMD_Parse will emit error messages as appropriate
		}
    }
    else
    {
      CMD_ShowUsages("broker","stumathews@gmail.com","a broker component");
      exit(0);
    }

    if(verbose_flag) PRINT("Broker starting.\n");

    NETINIT();
    
    main_event_loop();

    LIST_FreeInstance(settings);
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

    // NB: Getting a socket is always non-blocking
    s = netTcpServer(our_address,port);

    FD_SET(s, &readfds);

    do
    {
       if(verbose_flag) PRINT("** Listening.\n");

       /* wait/block on this listening socket... */
       int res = 0;

       if( waitIndef_flag )
          res =  select( s+1, &readfds, NULL, NULL, NULL);
       else
          res =  select( s+1, &readfds, NULL, NULL, &timeout);

        if( res == 0 )
        {
            LOG( "broker listen timeout!" );
            netError(1,errno, "timeout!" );
        }
        else if( res == -1 )
        {
            LOG( "Select error!" );
            netError( 1,errno, "select error!!" );
        }
        else
        {
            peerlen = sizeof( peer );

            if( FD_ISSET(s,&readfds ))
            {
                s1 = accept( s, ( struct sockaddr * )&peer, &peerlen );

                if ( !isvalidsock( s1 ) )
                    netError( 1, errno, "accept failed" );
                
                // Data arrived,  Process it
                server( s1, &peer );
                NETCLOSE( s1 );
            }
            else
            {
                DBG("Not our socket. continuing listening");
                continue;
            }
        }
    } while ( 1 );
}


/**
 * @brief Waits for connections received from clients and servers.
 * 
 * @param s The socket
 * @param peerp the peerlen
 * @return void
 */
static void server( SOCKET s, struct sockaddr_in *peerp )
{

    struct Packet packet;

    // 1. Read the size of packet.
    int n_rc = netReadn( s,(char*) &packet.len, sizeof(uint32_t));
    packet.len = ntohl(packet.len);

    if( n_rc < 1 ) netError(1, errno, "Failed to receiver packet size\n");
    if( verbose_flag ) PRINT("Received %d bytes and interpreted it as length of %u\n", n_rc,packet.len );
    
    packet.buffer = (char*) Alloc( sizeof(char) * packet.len);

    // 2. Read the packet data
    int d_rc  = netReadn( s, packet.buffer, sizeof( char) * packet.len);

    if( d_rc < 1 )  netError(1, errno,"failed to receive message\n");
    if(verbose_flag) PRINT("Read %d bytes of data.\n",d_rc);

    int request_type = -1; // default -1 represents invalid state
   
    if( (request_type = determine_request_type(&packet)) == REQUEST_SERVICE )
    {
        Location *src = get_sender_address( &packet, peerp );
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
