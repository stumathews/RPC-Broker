#include <stulibc.h>
#include "broker_support.h"
#include "../config.h"

#define SetupTCPServerSocket(our_address, port) netTcpServer((our_address), (port))

struct ServiceRegistration service_repository;
struct ClientRequestRegistration client_request_repository;

static const char* CONFIG_FILENAME = "config.ini";
static void main_event_loop();
static void server( SOCKET s, struct sockaddr_in *peerp, struct BrokerConfig *brokerConfig, struct BrokerDetails *brokerDetails );
void* thread_server(void* param);

void GetVerboseConfigSetting(struct BrokerConfig *brokerConfig, List* settings) {
	// if successful parse
	char* arg = INI_GetSetting(settings, "options", "verbose");
	if (STR_Equals(arg, "true") || STR_Equals(arg, "1")) {
		brokerConfig->verbose = true;
	} else {
		brokerConfig->verbose = false;
	}
}

void GetWaitIndefConfigSetting(struct BrokerConfig *brokerConfig, List* settings)
{
	char* result = INI_GetSetting(settings, "options", "wait");

	if(STR_EqualsIgnoreCase(result, "true") || STR_Equals(result, "1")){
		brokerConfig->waitIndef = true;
	} else {
		brokerConfig->waitIndef = false;
	}
}

void GetBrokerAddressConfigSetting(struct BrokerDetails* brokerDetails, List* settings)
{
	strncpy(brokerDetails->port,INI_GetSetting(settings, "networking", "port"),MAX_PORT_CHARS);
	printf("broker port is : %s", brokerDetails->port);
}

void GetBrokerPortConfigSettings(struct BrokerDetails* brokerDetails, List* settings)
{
	strncpy(brokerDetails->broker_address, INI_GetSetting(settings, "networking", "address"), MAX_ADDRESS_CHARS);
	printf("broker address is : %s", brokerDetails->broker_address);
}

/**
 * @brief see broker_support.c for additional functions used in the broker code
 * 
 * @param argc Number of args
 * @param argv args
 * @return int
 */
int main(int argc, char **argv)
{
    LIB_Init();

    INIT_LIST_HEAD(&service_repository.list);
    INIT_LIST_HEAD(&client_request_repository.list);

    struct BrokerConfig brokerConfig = {0};
    struct BrokerDetails brokerDetails = {0};

    /*
    struct Argument* cmdPort = CMD_CreateNewArgument("p", "p <number>", "Set the port that the broker will listen on",true, true, setPortNumber);
    struct Argument* cmdVerbose = CMD_CreateNewArgument("v","","Prints all messages verbosely",false,false,setVerboseFlag);
    struct Argument* cmdWaitIndef = CMD_CreateNewArgument("w","","Wait indefinitely for new connections, else 60 secs and then dies",false,false,setWaitIndefinitelyFlag);
    struct Argument* cmdMyAddress = CMD_CreateNewArgument("a","a <address>","Set our address",true, true, setOurAddress);
*/
    List* settings = {0};
/*
    CMD_AddArgument(cmdWaitIndef);
    CMD_AddArgument(cmdPort);
    CMD_AddArgument(cmdVerbose);
    CMD_AddArgument(cmdMyAddress);
*/
    if (FILE_Exists(CONFIG_FILENAME) && !(argc > 1)) {
    	DBG("Using config file located in '%s'", CONFIG_FILENAME);
    	settings = LIST_GetInstance();
    	if(INI_IniParse(CONFIG_FILENAME, settings) == 0) { // if successful parse
			GetVerboseConfigSetting(&brokerConfig, settings);
			GetWaitIndefConfigSetting(&brokerConfig, settings);
			GetBrokerAddressConfigSetting(&brokerDetails, settings);
			GetBrokerPortConfigSettings(&brokerDetails, settings);
		} else 	{
				ERR_Print("Failed to parse config file", 1);
		}
    }/*
    else if(argc > 1) {
    	if((CMD_Parse(argc, argv, true) != PARSE_SUCCESS)) {
			PRINT("CMD line parsing failed.");
			return 1;  // Note CMD_Parse will emit error messages as appropriate
		}
    }*/ else {
      CMD_ShowUsages("broker","stumathews@gmail.com","a broker component");
      exit(0);
    }

    if(brokerConfig.verbose) {
    	PRINT("Broker starting.\n");
    }

    NETINIT();
    
    main_event_loop(&brokerConfig, &brokerDetails);

    LIST_FreeInstance(settings);
    LIB_Uninit();

#ifdef __linux__
	pthread_exit(NULL);
#endif
    EXIT( 0 );
}


/**
 * @brief Setup networking port and continually waits for network service registrations from services,  and service requests from clients.
 * 
 * @return void
 */
static void main_event_loop(struct BrokerConfig *brokerConfig, struct BrokerDetails *brokerDetails)
{
    struct sockaddr_in local;
    
    char *hname;
    char *sname;
    
    SOCKET s;
    
    fd_set readfds;
    FD_ZERO( &readfds);
    const int on = 1;
    struct timeval timeout = {.tv_sec = 60, .tv_usec=0}; 

    // NB: Getting a socket is always non-blocking
    s = SetupTCPServerSocket(brokerDetails->broker_address, brokerDetails->port);

    FD_SET(s, &readfds);

    do
    {
       /* wait/block on this listening socket... */
       int res = 0;

       if(brokerConfig->waitIndef) {
          res =  select(s+1, &readfds, NULL, NULL, NULL);
       } else {
          res =  select(s+1, &readfds, NULL, NULL, &timeout);
       }
       
       if(brokerConfig->verbose) {
    	   PRINT("-- Listening...\n");
	   }

        if(res == 0) {
            LOG( "broker listen timeout!" );
            netError(1,errno, "timeout!" );
        }
        else if(res == -1) {
            LOG("Select error!");
            netError(1,errno, "select error!!");
        } else {
            if(FD_ISSET(s,&readfds)) {
            	if(brokerConfig->verbose) {
            		PRINT("++ Connection.\n");
            	}
            	struct BrokerServerArgs args = {0};
            		args.brokerConfig = brokerConfig;
            		args.brokerDetails = brokerDetails;
            		args.socket = &s;

            		// Fork of a new thread to deal with this request and go back to listening for next request
					#ifdef __linux__
							THREAD_RunAndForget(thread_server, (void*)&args);
					#else
							// Right now starting this broker as a thread causes problems.
							// In most cases this is probaly because its sharing state and I'm not doing
							// anything particualr to prevent it.
							thread_server((void*)&args);
							//THREAD_RunAndForget(thread_server, (void*)&s);
					#endif
            } else {
                DBG("Not our socket. continuing listening");
                continue;
            }
        }
    } while (1);
}

/***
 * Wrapper function to accept connection and process it - so that it confirms to void* func(void*) prototype so can pass as a thread function
 *
 * @param params SOCKET* socket that is ready to read from
 */
void* thread_server(void* params)
{
	struct BrokerServerArgs *args = (struct BrokerServerArgs*)params;
	SOCKET* s = (SOCKET*) args->socket;
	int peerlen;

	struct sockaddr_in peer;
	peerlen = sizeof(peer);

	SOCKET s1 = accept(*s,(struct sockaddr *)&peer, &peerlen);
	if (!isvalidsock(s1)) {
	    netError(1, errno, "accept failed");
	}
	// Data arrived,  Process it
	server(s1, &peer, args->brokerConfig, args->brokerDetails);
	NETCLOSE(s1);
	return (void*)0;
}

/**
 * @brief Processes the connection data
 * 
 * @param s The socket
 * @param peerp the peerlen
 * @return void
 */
static void server(SOCKET s, struct sockaddr_in *peerp, struct BrokerConfig *brokerConfig, struct BrokerDetails *brokerDetails )
{
    struct Packet packet;

    int packet_size = netReadn(s,(char*)&packet.len, sizeof(uint32_t));
    packet.len = ntohl(packet.len);

    if(packet_size < 1) {
    	netError(1, errno, "Failed to receiver packet size\n");
    }

    DBG("Got: %d bytes(packet length).", packet_size);
    DBG("Packet length of %u\n",packet.len );

    packet.buffer = (char*) malloc( sizeof(char) * packet.len);
    int data_size  = netReadn(s, packet.buffer, sizeof(char) * packet.len);

    if(data_size < 1) {
    	netError(1, errno,"failed to receive message\n");
    }

    int request_type = -1; // default -1 represents invalid state

    if(brokerConfig->verbose) {
    	PRINT("Got %d bytes [%d+%d] : " ,data_size + packet_size, packet_size, data_size);
    }

    if((request_type = determine_request_type(&packet)) == SERVICE_REQUEST)  {
    	char* operation = get_header_str_value(&packet, OPERATION_HDR);

    	if(brokerConfig->verbose) {
    		printf("SERVICE_REQUEST(%s)\n", operation);
    	}

        Location *src = malloc(sizeof(Location));
		get_sender_address(&packet, peerp, src);
        forward_request_to_server(&packet, src, brokerConfig);

        free(operation);
    } 
    else if (request_type == SERVICE_REGISTRATION) {
    	if(brokerConfig->verbose) {
    		printf("SERVICE_REGISTRATION\n");
    	}
        register_service_request(&packet, brokerConfig);
    } else if(request_type == SERVICE_REQUEST_RESPONSE) {
    	if(brokerConfig->verbose) {
    		printf("SERVICE_REQUEST_RESPONSE(%s)\n", get_header_str_value(&packet, OPERATION_HDR));
    	}
        Packet* response = &packet;
        forward_response_to_client(response, brokerConfig);
    } else {
    	PRINT("Unrecongnised request type:%d. Ignoring \n", request_type);
    }

    free(packet.buffer);
    return;
}
