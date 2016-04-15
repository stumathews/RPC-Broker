#include <stulibc.h>
#include "broker_support.h"
#include "../config.h"
#include "broker.h"
#include "common/common.h"
static struct Config brokerConfig = { 0 };
static struct Details brokerDetails = { 0 };

int main(int argc, char **argv) {
	LIB_Init();
	LIST_Init (&service_repository);
	List* settings = { 0 };
	
	struct Argument* cmdPort = CMD_CreateNewArgument("p", "p <number>",	"Set the port that the broker will listen on", true, true, 	setPortNumber);
	struct Argument* cmdVerbose = CMD_CreateNewArgument("v", "","Prints all messages verbosely", false, false, setVerboseFlag);
	struct Argument* cmdWaitIndef = CMD_CreateNewArgument("w", "","Wait indefinitely for new connections, else 60 secs and then dies",false, false, setWaitIndefinitelyFlag);
	
	CMD_AddArgument(cmdWaitIndef);
	CMD_AddArgument(cmdPort);
	CMD_AddArgument(cmdVerbose);
	
	if (FILE_Exists(CONFIG_FILENAME) && !(argc > 1)) {
		DBG("Using config file located in '%s'", CONFIG_FILENAME);
		settings = LIST_GetInstance();
		if (INI_IniParse(CONFIG_FILENAME, settings) == INI_PARSE_SUCCESS) {
			GetVerboseConfigSetting(&brokerConfig, settings);
			GetWaitIndefConfigSetting(&brokerConfig, settings);
			GetBrokerAddressConfigSetting(&brokerDetails, settings);
			GetBrokerPortConfigSettings(&brokerDetails, settings);
		} else {
			ERR_Print("Failed to parse config file", 1);
		}
	} else if (argc > 1) {
		if ((CMD_Parse(argc, argv, true) != PARSE_SUCCESS)) {
			PRINT("CMD line parsing failed.");
			return 1;  // Note CMD_Parse will emit error messages as appropriate
		}
	} else {
		CMD_ShowUsages("broker", "stumathews@gmail.com", "a broker component");
		exit(0);
	}

	if (brokerConfig.verbose) {
		PRINT("Broker starting.\n");
	}

	wait_for_connections(&brokerConfig, &brokerDetails);

	LIST_FreeInstance(settings);
	LIB_Uninit();

#ifdef __linux__
	pthread_exit(NULL);
#endif
	EXIT(0);
}

/**
 * @brief Setup networking port and continually waits for network service registrations from services,  and service requests from clients.
 *
 * @return void
 */
static void wait_for_connections(struct Config *brokerConfig, struct Details *brokerDetails)
{		
	fd_set read_file_descriptors = {0};
	int wait_result = 0;
	struct timeval timeout = {.tv_sec = 60, .tv_usec = 0};
	SOCKET listening_socket = GetAServerSocket(brokerDetails->address, brokerDetails->port);
	struct ServerArgs *threadParams = malloc(sizeof(struct ServerArgs));

	threadParams->config = brokerConfig;
	threadParams->details = brokerDetails;
	threadParams->socket = &listening_socket;

	FD_ZERO(&read_file_descriptors);
	FD_SET(listening_socket, &read_file_descriptors);

	do {
		if ((wait_result = wait(brokerConfig, listening_socket, &read_file_descriptors, &timeout)) == _WAIT_TIMEOUT) {
			netError(1, errno, "timeout occured while waiting for incomming connections!");
		} else if (wait_result == WAIT_ERROR) {
			netError(1, errno, "select error!!");
		} else {
			if (FD_ISSET(listening_socket, &read_file_descriptors)) {
				#ifdef USE_THREADING
						THREAD_RunAndForget(read_socket_thread_wrapper, (void*) threadParams);
				#else
						read_socket_thread_wrapper((void*) threadParams);
				#endif
			} else {
				DBG("Not on our socket. continuing listening");
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
#ifdef __linux__
void* read_socket_thread_wrapper(void* params)
#else
unsigned long read_socket_thread_wrapper(void* params)
#endif
{
		struct ServerArgs *args = (struct ServerArgs*) params;
		SOCKET* listening_socket = (SOCKET*) args->socket;
		int peerlen;
		struct sockaddr_in peer;
		peerlen = sizeof(peer);
		SOCKET connected_socket = accept(*listening_socket, (struct sockaddr *) &peer, &peerlen);

		CheckValidSocket(connected_socket);
		read_socket(connected_socket, &peer, args->config, args->details);
		NETCLOSE(connected_socket);

		return GetGenericThreadResult();
}

/**
 * @brief Processes the connection data
 *
 * @param s The socket
 * @param peerp the peerlen
 * @return void
 */
static void read_socket(SOCKET connected_socket, struct sockaddr_in *peerp, struct Config *brokerConfig, struct Details *brokerDetails)
{
	struct Packet packet;
	Location *sender = NULL;
	ClientReg *crreg = NULL;
	Location *destination = NULL;
	char* requested_operation = NULL;
	int* message_id = NULL;
	char* operation = NULL;
	int recieved_packet_size = 0;
	int request_type = -1; // default -1 represents invalid state
	int recieved_data_size = 0;

	recieved_packet_size = netReadn(connected_socket, (char*)&packet.len, sizeof(uint32_t));

	packet.len = ntohl(packet.len);
	packet.buffer = (char*) malloc(sizeof(char) * packet.len);

	if (recieved_packet_size < 1) { netError(1, errno, "Failed to receiver packet size\n"); }

	if (brokerConfig->verbose){
		PRINT("Got: %d bytes(packet length).", recieved_packet_size);
		PRINT("Packet length of %u\n", packet.len);
	}

	recieved_data_size = netReadn(connected_socket, packet.buffer, packet.len);

	if (recieved_data_size < 1) { netError(1, errno, "failed to receive message\n"); }
	if (brokerConfig->verbose) { PRINT("Got %d bytes [%d+%d] : ", recieved_data_size + recieved_packet_size, recieved_packet_size,	recieved_data_size); }

	if ((request_type = determine_request_type(&packet)) == SERVICE_REQUEST) {
		operation = get_header_str_value(&packet, OPERATION_HDR);
		PRINT("<<< SERVICE_REQUEST(%s)\n", operation);

		sender = malloc(sizeof(Location));
		get_sender_address(&packet, peerp, sender);

		requested_operation = malloc(sizeof(char));
		message_id = malloc(sizeof(int));

		*message_id = get_header_int_value(&packet, MESSAGE_ID_HDR);
		requested_operation = get_header_str_value(&packet, OPERATION_HDR);
		crreg = register_client_request(requested_operation, sender, *message_id, brokerConfig);
		destination = find_server_for_request(&packet, brokerConfig);

		if (destination->address == NULL || destination->port == NULL) {
			PRINT("No registered server available to process request for operation '%s' from host '%s'. \n", requested_operation, sender->address);
			return;
		}

		if (brokerConfig->verbose) { PRINT(">>> About to forward message to %s:%s\n", destination->address,	destination->port); }

		send_request(&packet, destination->address, destination->port, brokerConfig->verbose);
	} else if (request_type == SERVICE_REGISTRATION) {
		PRINT("<<< SERVICE_REQUEST(%s)\n", get_header_str_value(&packet, OPERATION_HDR));
		register_service_request(&packet, brokerConfig);
	} else if (request_type == SERVICE_REQUEST_RESPONSE) {
		PRINT("<< SERVICE_REQUEST_RESPONSE(%s)\n", get_header_str_value(&packet, OPERATION_HDR));
		Packet* response = &packet;
		PRINT(">>> FWD SERVICE_REQUEST_RESPONSE\n", get_header_str_value(&packet, OPERATION_HDR));
		forward_response_to_client(response, brokerConfig);
	} else {
		PRINT("Unrecognised request type:%d. Ignoring \n", request_type);
	}
	return;
}

void GetVerboseConfigSetting(struct Config *brokerConfig, List* settings) {
char* arg = INI_GetSetting(settings, "options", "verbose");
setVerboseFlag(arg);
}

void setVerboseFlag(char *verbose) {
char* arg = verbose;
if (STR_Equals(arg, "true") || STR_Equals(arg, "1")) {
	brokerConfig.verbose = true;
} else {
	brokerConfig.verbose = false;
}
}

void GetWaitIndefConfigSetting(struct Config *brokerConfig, List* settings) {
char* result = INI_GetSetting(settings, "options", "wait");
setWaitIndefinitelyFlag(result);
}

void setWaitIndefinitelyFlag(char *arg) {
if (STR_EqualsIgnoreCase(arg, "true") || STR_Equals(arg, "1")) {
	brokerConfig.waitIndef = true;
} else {
	brokerConfig.waitIndef = false;
}
}

void GetBrokerAddressConfigSetting(struct Details* brokerDetails, List* settings)
{
		strncpy(brokerDetails->port, INI_GetSetting(settings, "networking", "port"),
		MAX_PORT_CHARS);
}

void GetBrokerPortConfigSettings(struct Details* brokerDetails,	List* settings)
{
	strncpy(brokerDetails->address,		INI_GetSetting(settings, "networking", "address"), MAX_ADDRESS_CHARS);
}

void setPortNumber(char *arg) {
	strncpy(brokerDetails.address, arg, MAX_ADDRESS_CHARS);
}
int wait(struct Config *brokerConfig, SOCKET listening_socket, fd_set *read_file_descriptors, struct timeval *timeout)
{
	if (brokerConfig->verbose) PRINT("-- Listening...\n");

	if (brokerConfig->waitIndef) {
			return select(listening_socket + 1, read_file_descriptors, NULL, NULL, NULL);
	} else {
			return select(listening_socket + 1, read_file_descriptors, NULL, NULL, timeout);
	}
}

