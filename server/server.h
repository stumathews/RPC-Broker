/*
 * server.h
 *
 *  Created on: 30 Mar 2016
 *      Author: Stuart
 */

#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#define CONFIG_FILENAME "config.ini"
static Details brokerDetails = { };
static Details serverDetails = { };
static Config serverConfig = { };

static bool registered_with_broker = false;

/* Function prototypes */

bool service_register_with_broker(Details brokerDetails, Details serverDetails,
		Config brokerConfig);
void unpack_marshal_call_send(char* buffer, int buflen, Details brokerDetails,
		Config brokerConfig);
static void setBrokerPort(char* arg);
static void setPortNumber(char* arg);
static void setBrokerAddress(char* arg);
static void setOurAddress(char* arg);
static void setWaitIndef(char* arg);
static void setBeVerbose(char* arg);

#ifdef __linux__
void* thread_server(void* params);
#else
unsigned __stdcall thread_server(void* params);
#endif

static void ReadAndProcessDataOnSocket(SOCKET s, struct sockaddr_in *peerp);

inline void PrintConfigDiagnostics(_Bool verbose, List* settings) {
	if (verbose) {
		LIST_ForEach(settings, printSetting);
	}
}

#endif /* SERVER_SERVER_H_ */
