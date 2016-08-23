#include <stulibc.h>
#include "broker_support.h"
#include "../config.h"
#include "broker.h"
#include "common/common.h"
#include "sqlite3.h"

LockPtr lock;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
    for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

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

	struct Config config = { 0 };
	struct Details details = { 0 };

	config.svc_repo = &svc_repo;
	config.clnt_req_repo = &clnt_req_repo;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc = sqlite3_open("broker.db", &db);
	if( rc ){
	     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	     sqlite3_close(db);
	     return(1);
	 }
	 char* query = "CREATE TABLE IF NOT EXISTS names (id INTEGER PRIMARY KEY, name TEXT);\
			 INSERT INTO names (name) VALUES('Stuart');\
			 SELECT * FROM names;";
	 rc = sqlite3_exec(db, query, callback, 0, &zErrMsg);
	 if( rc!=SQLITE_OK ){
		 fprintf(stderr, "SQL error: %s\n", zErrMsg);
		 sqlite3_free(zErrMsg);
	 }

	 sqlite3_close(db);

	if (FILE_Exists(CONFIG_FILENAME) && !(argc > 1)) {
		if (INI_IniParse(CONFIG_FILENAME, &settings) == INI_PARSE_SUCCESS) {
			DBG("Using config file located in '%s'", CONFIG_FILENAME);
			get_verbose_setting(&config, &settings);
			get_wait_setting(&config, &settings);
			get_address_setting(&details, &settings);
			get_port_setting(&details, &settings);
		} else {
			ERR_Print("Failed to parse config file", 1);
		}
	} else if (argc > 1) {
		PRINT("CMD line parsing no longer supported.");
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
	Location* client = NULL;
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
		client = malloc(sizeof(Location));
		client = find_client_for_response(&packet, client, config);
		send_req(&packet, client->address, client->port, false);
	} else {
		PRINT("Unrecognised request type:%d. Ignoring \n", req_type);
	}
	return;
}

int IsTrueString(char* arg) {
	return STR_EqualsIgnoreCase(arg, "1") || STR_EqualsIgnoreCase(arg, "true");
}

void get_verbose_setting(struct Config *config, List* settings)
{
	char* arg = INI_GetSetting(settings, "options", "verbose");
	config->verbose = IsTrueString(arg);
}

void get_wait_setting(struct Config *config, List* settings)
{
	char* result = INI_GetSetting(settings, "options", "wait");
	config->waitIndef = IsTrueString(result);
}

void get_address_setting(struct Details* detail, List* settings)
{
	strncpy(detail->port, INI_GetSetting(settings, "networking", "port"), MAX_PORT_CHARS);
}

void get_port_setting(struct Details* details,	List* settings)
{
	strncpy(details->address, INI_GetSetting(settings, "networking", "address"), MAX_ADDRESS_CHARS);
}

int wait_rd_socket(struct Config *config, SOCKET listening_socket, fd_set *rd_fds, struct timeval *timeout)
{
	if (config->waitIndef)
		return select(listening_socket + 1, rd_fds, NULL, NULL, NULL);
	else
		return select(listening_socket + 1, rd_fds, NULL, NULL, timeout);
}
