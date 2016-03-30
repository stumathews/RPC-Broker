/*
 * broker.h
 *
 *  Created on: 30 Mar 2016
 *      Author: stuartm
 */

#ifndef BROKER_BROKER_H_
#define BROKER_BROKER_H_

#define SetupTCPServerSocket(our_address, port) netTcpServer((our_address), (port))
#define INI_PARSE_SUCCESS 0
#define MAX_FILENAME_LENGTH 255

struct ServiceRegistration service_repository;
struct ClientRequestRegistration client_request_repository;
static const char* CONFIG_FILENAME = "config.ini";

#ifdef __linux__
void* thread_server(void* params);
#else
unsigned long thread_server(void* params);
#endif

static void main_event_loop();
static void server(SOCKET s, struct sockaddr_in *peerp, struct Config *brokerConfig, struct Details *brokerDetails);
void GetVerboseConfigSetting(struct Config *brokerConfig, List* settings);
void setVerboseFlag(char *verbose);
void GetWaitIndefConfigSetting(struct Config *brokerConfig, List* settings);
void setWaitIndefinitelyFlag(char *arg);
void GetBrokerAddressConfigSetting(struct Details* brokerDetails, List* settings);
void GetBrokerPortConfigSettings(struct Details* brokerDetails, List* settings);
void setPortNumber(char *arg);

#endif /* BROKER_BROKER_H_ */
