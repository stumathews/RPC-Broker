#include <stulibc.h>
#include "common.h"
#include "server_interface.h"
#include "server.h"

int main(int argc, char **argv)
{
	struct sockaddr_in peer;
	int peerlen;
	SOCKET listening_socket;
	fd_set readfds;
	FD_ZERO(&readfds);
	int wait_result = -1;

	LIB_Init();
	PRINT("Server starting...");

	struct Argument* portNumber = CMD_CreateNewArgument("p", "p <number>",
			"Set the port that the server will listen on", true, true,
			setPortNumber);
	struct Argument* brokerAddressCMD = CMD_CreateNewArgument("ba",
			"ba <address>", "Set the address of the broker", true, true,
			setBrokerAddress);
	struct Argument* ourAddressCMD = CMD_CreateNewArgument("oa", "oa <address>",
			"Set the address we listen on (broker uses to deliver responses)",
			true, true, setOurAddress);
	struct Argument* brokerPortCMD = CMD_CreateNewArgument("bp", "bp <port>",
			"Set the port of the broker", true, true, setBrokerPort);
	struct Argument* waitIndefCMD = CMD_CreateNewArgument("w", "",
			"wait indefinitely for connections", false, false, setWaitIndef);
	struct Argument* beVerboseCMD = CMD_CreateNewArgument("v", "",
			"be verbose wtih messages", false, false, setBeVerbose);

	CMD_AddArgument(portNumber);
	CMD_AddArgument(waitIndefCMD);
	CMD_AddArgument(brokerAddressCMD);
	CMD_AddArgument(brokerPortCMD);
	CMD_AddArgument(beVerboseCMD);
	CMD_AddArgument(ourAddressCMD);

	List* settings = (void*) 0;
	struct timeval timeout = {.tv_sec = 60, .tv_usec = 0};

	if (FILE_Exists(CONFIG_FILENAME) && (argc < 2)) {
		DBG("Using config file located in '%s'", CONFIG_FILENAME);
		settings = LIST_GetInstance();
		if (INI_IniParse(CONFIG_FILENAME, settings) == 0) { // if successful parse
			setWaitIndef(INI_GetSetting(settings, "options", "wait"));
			setBeVerbose(INI_GetSetting(settings, "options", "verbose"));
			setPortNumber(INI_GetSetting(settings, "networking", "port"));
			setOurAddress(INI_GetSetting(settings, "networking", "address"));
			setBrokerAddress(INI_GetSetting(settings, "broker", "address"));
			setBrokerPort(INI_GetSetting(settings, "broker", "port"));
			PrintConfigDiagnostics(serverConfig.verbose, settings);
		} else {
			ERR_Print("Failed to parse config file", 1);
		}
	} else if (argc > 1) {
		enum ParseResult result = CMD_Parse(argc, argv, true);
		if (result != PARSE_SUCCESS) {
			PRINT("There was a problem parsing: %d \n", result);
			return 1;
		}
	} else {
		CMD_ShowUsages("server", "stumathews@gmail.com", "the server component");
		exit(0);
	}

	NETINIT();

	if(serverConfig.verbose) {
		PRINT("Server listening...\n");
		PRINT("Register with the broker on startup.\n");
		PRINT("broker address is %s, broker port is %s \n", brokerDetails.address,brokerDetails.port);
		PRINT("Sending registration request to broker at address '%s:%s'", brokerDetails.address, brokerDetails.port);
	}
	service_register_with_broker(brokerDetails, serverDetails,serverConfig);

	if(serverConfig.verbose) {PRINT("Wait for messages from the broker...\n");}
	listening_socket = netTcpServer(serverDetails.address, serverDetails.port);
	FD_SET(listening_socket, &readfds);
	struct ServerArgs *threadParams = malloc(sizeof(struct ServerArgs));
	*threadParams = (struct ServerArgs) {.config = &serverConfig, .details = &serverDetails, .socket = &listening_socket};

	do {
		if ((wait_result = wait(&serverConfig, listening_socket, &readfds, &timeout)) == _WAIT_TIMEOUT) {
			LOG("timeout");
			netError(1, errno, "timeout!");
		} else if (wait_result == WAIT_ERROR) {
			LOG("Select error!");
			netError(1, errno, "select error!!");
		} else {
			peerlen = sizeof(peer);
			if (FD_ISSET(listening_socket, &readfds)) {
				THREAD_RunAndForget(thread_server, (void*) threadParams);
			} else {
				DBG("not our socket. continuing");
				continue;
			}
		}
	} while (1);

	LIST_FreeInstance (settings);
	LIB_Uninit();
	#ifdef __linux__
	pthread_exit(NULL);
	#endif
	EXIT(0);
}

#ifdef __linux__
void* thread_server(void* params);
#else
unsigned __stdcall thread_server(void* params)
#endif
{
	int peerlen;
	struct sockaddr_in peer;
	peerlen = sizeof(peer);
	struct ServerArgs* threadParams = (struct ServerArgs*) params;
	SOCKET* listening_socket = threadParams->socket;
	SOCKET connected_socket = accept(*listening_socket, (struct sockaddr*) &peer, &peerlen);
	CheckValidSocket(connected_socket);
	ReadAndProcessDataOnSocket(connected_socket, &peer, threadParams->config);
	NETCLOSE(connected_socket);

	return GetGenericThreadResult();
}

static void ReadAndProcessDataOnSocket(SOCKET connected_socket, struct sockaddr_in *peerp, struct Config* config)
{
	Packet pkt;
	int request_type = -1;
	int recieved_length = 0;
	int recieved_data_bytes = 0;
	static bool isRegistered = false;

	recieved_length = netReadn(connected_socket, (char*) &pkt.len,
			sizeof(uint32_t));

	pkt.len = ntohl(pkt.len);
	pkt.buffer = (char*) malloc(sizeof(char) * pkt.len);

	recieved_data_bytes = netReadn(connected_socket, pkt.buffer,
			sizeof(char) * pkt.len);

	if (config->verbose) { PRINT("received %d bytes and interpreted it as length of %u\n", recieved_length, pkt.len); }
	if (recieved_length < 1) { netError(1, errno, "failed to receiver packet size\n"); }
	if (recieved_data_bytes < 1) { netError(1, errno, "failed to receive message\n"); }
	if (config->verbose) { PRINT("read %d bytes of data\n", recieved_data_bytes); }

	unpack_data(&pkt, config->verbose);

	if ((request_type = determine_request_type(&pkt)) == SERVICE_REGISTRATION_ACK) {
		isRegistered = true;
		if (serverConfig.verbose) { PRINT("Registered with broker.\n"); }
	} else if (request_type == SERVICE_REQUEST && isRegistered) {
		unpack_marshal_call_send(pkt.buffer, pkt.len, brokerDetails, serverConfig);
	} else { PRINT("%s\n", 	!isRegistered ? "Not registered yet - waiting for registration ACK from broker" : "unknown message received from broker");	}
}
// ===============================
// Command line handling routines
// ===============================
static void setBrokerPort(char* arg) {
CHECK_STRING(arg, IS_NOT_EMPTY);
strncpy(brokerDetails.port, arg, strlen(arg));
}

static void setPortNumber(char* arg) {
CHECK_STRING(arg, IS_NOT_EMPTY);
strncpy(serverDetails.port, arg, strlen(arg));
}

static void setBrokerAddress(char* arg) {
CHECK_STRING(arg, IS_NOT_EMPTY);
strncpy(brokerDetails.address, arg, strlen(arg));
}
static void setOurAddress(char* arg) {
CHECK_STRING(arg, IS_NOT_EMPTY);
strncpy(serverDetails.address, arg, strlen(arg));
}

static void setWaitIndef(char* arg) {
serverConfig.waitIndef = true;
}

static void setBeVerbose(char* arg) {
if (STR_Equals(arg, "true") || STR_Equals(arg, "1")) {
	serverConfig.verbose = true;
} else {
	serverConfig.verbose = false;
}
}

int wait(struct Config *serverConfig, SOCKET listening_socket,
	fd_set *read_file_descriptors, struct timeval *timeout) {
if (serverConfig->verbose)
	PRINT("-- Listening...\n");

if (serverConfig->waitIndef) {
	return select(listening_socket + 1, read_file_descriptors, NULL, NULL, NULL);
} else {
	return select(listening_socket + 1, read_file_descriptors, NULL, NULL,
			timeout);
}
}

