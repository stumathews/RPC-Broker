/*
 * broker.h
 *
 *  Created on: 30 Mar 2016
 *      Author: stuartm
 */

#ifndef BROKER_BROKER_H_
#define BROKER_BROKER_H_

#define GetAServerSocket(our_address, port) netTcpServer((our_address), (port))
#define MAX_FILENAME_LENGTH 255

//struct ServiceRegistration service_repository;
List service_repository;
List client_request_repository;
static const char* CONFIG_FILENAME = "config.ini";


#ifdef USE_THREADING
#undef USE_THREADING
#endif

#ifdef __linux__
void* read_socket_thread_wrapper(void* params);
#else
unsigned long read_socket_thread_wrapper(void* params);
#endif

static void wait_for_connections();
static void read_socket(SOCKET s, struct sockaddr_in *peerp,
		struct Config *brokerConfig, struct Details *brokerDetails);
void GetVerboseConfigSetting(struct Config *brokerConfig, List* settings);
void setVerboseFlag(char *verbose);
void GetWaitIndefConfigSetting(struct Config *brokerConfig, List* settings);
void setWaitIndefinitelyFlag(char *arg);
void GetBrokerAddressConfigSetting(struct Details* brokerDetails,
		List* settings);
void GetBrokerPortConfigSettings(struct Details* brokerDetails, List* settings);
void setPortNumber(char *arg);
int wait(struct Config *brokerConfig, SOCKET listening_socket, fd_set *read_file_descriptors, struct timeval *timeout);

#endif /* BROKER_BROKER_H_ */
