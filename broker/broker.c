#include <stulibc.h>
#include "broker_support.h"
#include "../config.h"
#include "broker.h"
#include "common/common.h"

LockPtr lock;
struct Config config = { 0 };
struct Details details = { 0 };

/***
 * Read settings and wait for connections
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
	MakeLock(&lock);
	LIB_Init();

	List settings;
	List svc_repo;
	List clnt_req_repo;
	LIST_Init (&svc_repo);
	LIST_Init (&clnt_req_repo);
	LIST_Init (&settings);


	config.svc_repo = &svc_repo;
	config.clnt_req_repo = &clnt_req_repo;

	set_cmd_args();

	if (FILE_Exists(CONFIG_FILENAME) && !(argc > 1)) {
		DBG("Using config file located in '%s'", CONFIG_FILENAME);
		if (INI_IniParse(CONFIG_FILENAME, &settings) == INI_PARSE_SUCCESS) {
			get_verbose_setting(&config, &settings);
			get_wait_setting(&config, &settings);
			get_address_setting(&details, &settings);
			get_port_setting(&details, &settings);
		} else {
			ERR_Print("Failed to parse config file", 1);
		}
	} else if (argc > 1) {
		if ((CMD_Parse(argc, argv, true) != PARSE_SUCCESS)) {
			PRINT("CMD line parsing failed.");
			return 1;
		}
	} else {
		CMD_ShowUsages("broker", "stumathews@gmail.com", "a broker component");
		exit(0);
	}
	if (config.verbose) {
		PRINT("Broker starting.\n");
	}
	wait_for_connections(&config, &details);
	LIST_FreeInstance(&settings);
	LIST_FreeInstance(&svc_repo);
	LIST_FreeInstance(&clnt_req_repo);
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
static void wait_for_connections(struct Config *config, struct Details *details)
{
	fd_set rd_fd = {0};
	int wait_result = 0;
	struct timeval timeout = {.tv_sec = 60, .tv_usec = 0};
	SOCKET listening_socket = netTcpServer(details->address, details->port);

	do {
		FD_ZERO(&rd_fd);
		FD_SET(listening_socket, &rd_fd);
		struct ServerArgs *threadParams = malloc(sizeof(struct ServerArgs));

		threadParams->config = config;
		threadParams->details = details;
		threadParams->socket = &listening_socket;

		if ((wait_result = wait_rd_socket(config, listening_socket, &rd_fd, &timeout)) == _WAIT_TIMEOUT) {
			netError(1, errno, "timeout occured while waiting for incomming connections!");
		} else if (wait_result == WAIT_ERROR) {
			netError(1, errno, "select error!!");
		} else {
			if (FD_ISSET(listening_socket, &rd_fd)) {
				#ifdef USE_THREADING
						THREAD_RunAndForget(process_data_avail, (void*) threadParams);
				#else
						process_data_avail((void*) threadParams);
				#endif
			} else {
				DBG("Not on our socket. continuing listening");
				continue;
			}
		}
	} while (FOREVER);
}


/***
 * Wrapper function to accept connection and process it - so that it confirms to void* func(void*) prototype so can pass as a thread function
 *
 * @param params SOCKET* socket that is ready to read from
 */
THREADFUNC(process_data_avail)
{
	if(AquireLock(&lock))
	{
		struct ServerArgs *args = (struct ServerArgs*) params;
		SOCKET* listening_socket = (SOCKET*) args->socket;
		int peerlen;
		struct sockaddr_in peer;

		peerlen = sizeof(peer);

		SOCKET connected_socket = accept(*listening_socket, (struct sockaddr *) &peer, &peerlen);
		check_socket(connected_socket);

		read_data(connected_socket, &peer, args->config, args->details);

		NETCLOSE(connected_socket);
		free(params);

		ReleaseLock(&lock);
	} else {
		DBG("Could not aquire lock\n");
	}
	return THREAD_RESULT();
}

/**
 * @brief Processes the connection data
 *
 * @param s The socket
 * @param peerp the peerlen
 * @return void
 */
void read_data(SOCKET connected_socket, struct sockaddr_in *peer, struct Config *config, struct Details *details)
{
	struct Packet packet;
	Location *sender = NULL;
	ClientReg *clnt_reg = NULL;
	Location *dest = NULL;
	char* req_op = NULL;
	int msg_id;
	char* op = NULL;
	int pkt_size = 0;
	int req_type = -1;
	int rc_data_size = 0;

	sender = malloc(sizeof(Location));
	req_op = malloc(sizeof(char));

	pkt_size = netReadn(connected_socket, (char*)&packet.len, sizeof(uint32_t));

	packet.len = ntohl(packet.len);
	packet.buffer = (char*) malloc(sizeof(char) * packet.len);

	if (pkt_size < 1) {
		netError(1, errno, "Failed to receiver packet size\n");
	}

	if (config->verbose) {
		PRINT("Got: %d bytes(packet length).", pkt_size);
		PRINT("Packet length of %u\n", packet.len);
	}

	rc_data_size = netReadn(connected_socket, packet.buffer, packet.len);

	if (rc_data_size < 1) {
		netError(1, errno, "failed to receive message\n");
	}

	if (config->verbose) {
		PRINT("Got %d bytes [%d+%d] : ", rc_data_size + pkt_size, pkt_size,	rc_data_size);
		unpack_data(&packet, config->verbose);
	}

	if ((req_type = get_req_type(&packet)) == SERVICE_REQUEST) {
		op = get_hdr_str(&packet, OPERATION_HDR);
		get_sender_address(&packet, peer, sender);
		msg_id = get_hdr_int(&packet, MESSAGE_ID_HDR);
		req_op = get_hdr_str(&packet, OPERATION_HDR);
		clnt_reg = reg_clnt_req(req_op, sender, msg_id, config);
		dest = find_server_for_req(&packet, config);

		if (dest->address == NULL || dest->port == NULL) {
			PRINT("No registered server available to process request for op '%s' from host '%s'. \n", req_op, sender->address);
			return;
		}
		send_req(&packet, dest->address, dest->port, config->verbose);
	} else if (req_type == SERVICE_REGISTRATION) {
		reg_svc_req(&packet, config);
	} else if (req_type == SERVICE_REQUEST_RESPONSE) {
		Packet* response = &packet;
		fwd_to_clnt(response, config);
	} else {
		PRINT("Unrecognised request type:%d. Ignoring \n", req_type);
	}
	return;
}

void get_verbose_setting(struct Config *config, List* settings)
{
	char* arg = INI_GetSetting(settings, "options", "verbose");
	set_verbose(arg);
}

void set_verbose(char *verbose)
{
	char* arg = verbose;
	if (STR_Equals(arg, "true") || STR_Equals(arg, "1")) {
		config.verbose = true;
	} else {
		config.verbose = false;
	}
}

void get_wait_setting(struct Config *config, List* settings)
{
	char* result = INI_GetSetting(settings, "options", "wait");
	set_waitindef(result);
}

void set_waitindef(char *arg)
{
	if (STR_EqualsIgnoreCase(arg, "true") || STR_Equals(arg, "1")) {
		config.waitIndef = true;
	} else {
		config.waitIndef = false;
	}
}

void get_address_setting(struct Details* detail, List* settings)
{
	strncpy(detail->port, INI_GetSetting(settings, "networking", "port"),
	MAX_PORT_CHARS);
}

void get_port_setting(struct Details* details,	List* settings)
{
	strncpy(details->address, INI_GetSetting(settings, "networking", "address"), MAX_ADDRESS_CHARS);
}

void set_port_num(char *arg)
{
	PRINT("Brok3er addres is %s\n", details.address);
	strncpy(details.address, arg, MAX_ADDRESS_CHARS);
	DBG("EXIT:setPortNumber");
}

int wait_rd_socket(struct Config *config, SOCKET listening_socket, fd_set *rd_fds, struct timeval *timeout)
{
	if (config->waitIndef)
		return select(listening_socket + 1, rd_fds, NULL, NULL, NULL);
	else
		return select(listening_socket + 1, rd_fds, NULL, NULL, timeout);
}

void set_cmd_args()
{
	DBG("ENTRY: set_cmd_args");
	struct Argument* cmdPort = CMD_CreateNewArgument("p", "p <number>", "Set the port that the broker will listen on", true, true,	set_port_num);
	struct Argument* cmdVerbose = CMD_CreateNewArgument("v", "", "Prints all messages verbosely", false, false, set_verbose);
	struct Argument* cmdWaitIndef = CMD_CreateNewArgument("w", "", "Wait indefinitely for new connections, else 60 secs and then dies", false, false, set_waitindef);
	CMD_AddArgument(cmdWaitIndef);
	CMD_AddArgument(cmdPort);
	CMD_AddArgument(cmdVerbose);
	DBG("EXIT: set_cmd_args");
}
